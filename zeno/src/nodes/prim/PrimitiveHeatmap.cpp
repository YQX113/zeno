#include <zeno/zeno.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/types/StringObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/HeatmapObject.h>
#include <sstream>

namespace zeno {
struct MakeHeatmap : zeno::INode {
    virtual void apply() override {
        auto nres = get_param<int>("nres");
        auto ramps = get_param<std::string>("_RAMPS");
        std::stringstream ss(ramps);
        std::vector<std::pair<float, zeno::vec3f>> colors;
        int count;
        ss >> count;
        for (int i = 0; i < count; i++) {
            float f = 0.f, x = 0.f, y = 0.f, z = 0.f;
            ss >> f >> x >> y >> z;
            //printf("%f %f %f %f\n", f, x, y, z);
            colors.emplace_back(
                    f, zeno::vec3f(x, y, z));
        }

        auto heatmap = std::make_shared<HeatmapObject>();
        for (int i = 0; i < nres; i++) {
            float fac = i * (1.f / nres);
            zeno::vec3f clr;
            for (int j = 0; j < colors.size(); j++) {
                auto [f, rgb] = colors[j];
                if (f >= fac) {
                    if (j != 0) {
                        auto [last_f, last_rgb] = colors[j - 1];
                        auto intfac = (fac - last_f) / (f - last_f);
                        //printf("%f %f %f %f\n", fac, last_f, f, intfac);
                        clr = zeno::mix(last_rgb, rgb, intfac);
                    } else {
                        clr = rgb;
                    }
                    break;
                }
            }
            heatmap->colors.push_back(clr);
        }
        set_output("heatmap", std::move(heatmap));
    }
};

ZENDEFNODE(MakeHeatmap,
        { /* inputs: */ {
        }, /* outputs: */ {
        "heatmap",
        }, /* params: */ {
        {"int", "nres", "1024"},
        //{"string", "_RAMPS", "0 0 0.8 0.8 0.8 1"},
        }, /* category: */ {
        "visualize",
        }});


struct PrimitiveColorByHeatmap : zeno::INode {
    virtual void apply() override {
        auto prim = get_input<zeno::PrimitiveObject>("prim");
        auto heatmap = get_input<HeatmapObject>("heatmap");
        auto attrName = get_param<std::string>("attrName");
        float maxv = 1.0f;
        float minv = 0.0f;
        if(has_input("max"))
            maxv = get_input<NumericObject>("max")->get<float>();
        if(has_input("min"))
            minv = get_input<NumericObject>("min")->get<float>();
        auto &clr = prim->add_attr<zeno::vec3f>("clr");
        auto &src = prim->attr<float>(attrName);
        #pragma omp parallel for //ideally this could be done in opengl
        for (int i = 0; i < src.size(); i++) {
            auto x = (src[i]-minv)/(maxv-minv);
            // src[i] = (src[i]-minv)/(maxv-minv);
            clr[i] = heatmap->interp(x);
        }

        set_output("prim", std::move(prim));
    }
};

ZENDEFNODE(PrimitiveColorByHeatmap,
        { /* inputs: */ {
        "prim", "heatmap", {"float", "min", "0"}, {"float", "max", "1"},
        }, /* outputs: */ {
        "prim",
        }, /* params: */ {
        {"string", "attrName", "rho"},
        }, /* category: */ {
        "deprecated",
        }});
struct PrimSample1D : zeno::INode {
    virtual void apply() override {
        auto prim = get_input<PrimitiveObject>("prim");
        auto srcChannel = get_input2<std::string>("srcChannel");
        auto dstChannel = get_input2<std::string>("dstChannel");
        auto heatmap = get_input<HeatmapObject>("heatmap");
        auto remapMin = get_input2<float>("remapMin");
        auto remapMax = get_input2<float>("remapMax");
        primSampleHeatmap(prim, srcChannel, dstChannel, heatmap, remapMin, remapMax);

        set_output("outPrim", std::move(prim));
    }
};
ZENDEFNODE(PrimSample1D, {
    {
        {"PrimitiveObject", "prim"},
        {"heatmap"},
        {"string", "srcChannel", "rho"},
        {"string", "dstChannel", "clr"},
        {"float", "remapMin", "0"},
        {"float", "remapMax", "1"},
    },
    {
        {"PrimitiveObject", "outPrim"}
    },
    {},
    {"primitive"},
});
void primSampleHeatmap(
        std::shared_ptr<PrimitiveObject> prim,
        const std::string &srcChannel,
        const std::string &dstChannel,
        std::shared_ptr<HeatmapObject> heatmap,
        float remapMin,
        float remapMax
) {
    auto &clr = prim->add_attr<zeno::vec3f>(dstChannel);
    auto &src = prim->attr<float>(srcChannel);
#pragma omp parallel for //ideally this could be done in opengl
    for (int i = 0; i < src.size(); i++) {
        auto x = (src[i]-remapMin)/(remapMax-remapMin);
        clr[i] = heatmap->interp(x);
    }
}
}
