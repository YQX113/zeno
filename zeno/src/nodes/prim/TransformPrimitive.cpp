#include "zeno/types/UserData.h"
#include "zeno/funcs/ObjectGeometryInfo.h"
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <zeno/types/MatrixObject.h>
#include <zeno/types/NumericObject.h>
#include <zeno/types/PrimitiveObject.h>
#include <zeno/zeno.h>

namespace zeno {
namespace {

struct SetMatrix : zeno::INode{//ZHXX: use Assign instead!
    virtual void apply() override {
        auto &dst = std::get<glm::mat4>(get_input<zeno::MatrixObject>("dst")->m);
        auto &src = std::get<glm::mat4>(get_input<zeno::MatrixObject>("src")->m);
        dst = src;
    }
};
ZENDEFNODE(SetMatrix, {
    {
    {"dst" },
    {"src" },
    },
    {},
    {},
    {"math"},
});

struct MakeLocalSys : zeno::INode{
    virtual void apply() override {
        zeno::vec3f front = {1,0,0};
        zeno::vec3f up = {0,1,0};
        zeno::vec3f right = {0,0,1};
        if (has_input("front"))
            front = get_input<zeno::NumericObject>("front")->get<zeno::vec3f>();
        if (has_input("up"))
            up = get_input<zeno::NumericObject>("up")->get<zeno::vec3f>();
        if (has_input("right"))
            right = get_input<zeno::NumericObject>("right")->get<zeno::vec3f>();

        auto oMat = std::make_shared<MatrixObject>();
        oMat->m = glm::mat4(glm::mat3(front[0], up[0], right[0],
                            front[1], up[1], right[1],
                            front[2], up[2], right[2]));
        set_output("LocalSys", oMat);                    
    }
};
ZENDEFNODE(MakeLocalSys, {
    {
    {"vec3f", "front", "1,0,0"},
    {"vec3f", "up", "0,1,0"},
    {"vec3f", "right", "0,0,1"},
    },
    {{"LocalSys"}},
    {},
    {"math"},
});

struct TransformPrimitive : zeno::INode {//zhxx happy node
    static glm::vec3 mapplypos(glm::mat4 const &matrix, glm::vec3 const &vector) {
        auto vector4 = matrix * glm::vec4(vector, 1.0f);
        return glm::vec3(vector4) / vector4.w;
    }


    static glm::vec3 mapplynrm(glm::mat4 const &matrix, glm::vec3 const &vector) {
        glm::mat3 normMatrix(matrix);
        normMatrix = glm::transpose(glm::inverse(normMatrix));
        auto vector3 = normMatrix * vector;
        return glm::normalize(vector3);
    }

    virtual void apply() override {
        zeno::vec3f translate = {0,0,0};
        zeno::vec4f rotation = {0,0,0,1};
        zeno::vec3f eulerXYZ = {0,0,0};
        zeno::vec3f scaling = {1,1,1};
        zeno::vec3f shear = {0,0,0};
        zeno::vec3f offset = {0,0,0};
        glm::mat4 pre_mat = glm::mat4(1.0);
        glm::mat4 pre_apply = glm::mat4(1.0);
        glm::mat4 local = glm::mat4(1.0);
        if (has_input("Matrix"))
            pre_mat = std::get<glm::mat4>(get_input<zeno::MatrixObject>("Matrix")->m);
        if (has_input("translation"))
            translate = get_input<zeno::NumericObject>("translation")->get<zeno::vec3f>();
        if (has_input("eulerXYZ"))
            eulerXYZ = get_input<zeno::NumericObject>("eulerXYZ")->get<zeno::vec3f>();
        if (has_input("quatRotation"))
            rotation = get_input<zeno::NumericObject>("quatRotation")->get<zeno::vec4f>();
        if (has_input("scaling"))
            scaling = get_input<zeno::NumericObject>("scaling")->get<zeno::vec3f>();
        if (has_input("shear"))
            shear = get_input<zeno::NumericObject>("shear")->get<zeno::vec3f>();
        if (has_input("offset"))
            offset = get_input<zeno::NumericObject>("offset")->get<zeno::vec3f>();
        if (has_input("local"))
           local = std::get<glm::mat4>(get_input<zeno::MatrixObject>("local")->m);
        if (has_input("preTransform"))
            pre_apply = std::get<glm::mat4>(get_input<zeno::MatrixObject>("preTransform")->m);

        glm::mat4 matTrans = glm::translate(glm::vec3(translate[0], translate[1], translate[2]));
        glm::mat4 matRotx  = glm::rotate( eulerXYZ[0], glm::vec3(1,0,0) );
        glm::mat4 matRoty  = glm::rotate( eulerXYZ[1], glm::vec3(0,1,0) );
        glm::mat4 matRotz  = glm::rotate( eulerXYZ[2], glm::vec3(0,0,1) );
        glm::quat myQuat(rotation[3], rotation[0], rotation[1], rotation[2]);
        glm::mat4 matQuat  = glm::toMat4(myQuat);
        glm::mat4 matScal  = glm::scale( glm::vec3(scaling[0], scaling[1], scaling[2] ));
        glm::mat4 matShearX = glm::transpose(glm::mat4(
            1, shear[0], 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1));
        glm::mat4 matShearY = glm::transpose(glm::mat4(
            1, 0, shear[1], 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1));
        glm::mat4 matShearZ = glm::transpose(glm::mat4(
            1, 0, 0, 0,
            0, 1, shear[2], 0,
            0, 0, 1, 0,
            0, 0, 0, 1));

        auto matrix = pre_mat*local*matTrans*matRotz*matRoty*matRotx*matQuat*matScal*matShearZ*matShearY*matShearX*glm::translate(glm::vec3(offset[0], offset[1], offset[2]))*glm::inverse(local)*pre_apply;

        auto prim = get_input<PrimitiveObject>("prim");
        auto outprim = std::make_unique<PrimitiveObject>(*prim);

        if (prim->has_attr("pos")) {
            auto &pos = outprim->attr<zeno::vec3f>("pos");
            #pragma omp parallel for
            for (int i = 0; i < pos.size(); i++) {
                auto p = zeno::vec_to_other<glm::vec3>(pos[i]);
                p = mapplypos(matrix, p);
                pos[i] = zeno::other_to_vec<3>(p);
            }
        }

        if (prim->has_attr("nrm")) {
            auto &nrm = outprim->attr<zeno::vec3f>("nrm");
            #pragma omp parallel for
            for (int i = 0; i < nrm.size(); i++) {
                auto n = zeno::vec_to_other<glm::vec3>(nrm[i]);
                n = mapplynrm(matrix, n);
                nrm[i] = zeno::other_to_vec<3>(n);
            }
        }

        auto& user_data = outprim->userData();
        user_data.setLiterial("_translate", translate);
        vec4f rotate = {myQuat.x, myQuat.y, myQuat.z, myQuat.w};
        user_data.setLiterial("_rotate", rotate);
        user_data.setLiterial("_scale", scaling);
        //auto oMat = std::make_shared<MatrixObject>();
        //oMat->m = matrix;
        set_output("outPrim", std::move(outprim));
    }
};

ZENDEFNODE(TransformPrimitive, {
    {
    {"PrimitiveObject", "prim"},
    {"vec3f", "translation", "0,0,0"},
    {"vec3f", "eulerXYZ", "0,0,0"},
    {"vec4f", "quatRotation", "0,0,0,1"},
    {"vec3f", "scaling", "1,1,1"},
    {"vec3f", "shear", "0,0,0"},
    {"Matrix"},
    {"preTransform"},
    {"local"},
    },
    {
    {"PrimitiveObject", "outPrim"}
    },
    {},
    {"primitive"},
});

// euler rot order: roll-pitch-yaw
// euler rot unit use degrees
struct PrimitiveTransform : zeno::INode {
    static glm::vec3 mapplypos(glm::mat4 const &matrix, glm::vec3 const &vector) {
        auto vector4 = matrix * glm::vec4(vector, 1.0f);
        return glm::vec3(vector4) / vector4.w;
    }

    static glm::vec3 mapplynrm(glm::mat4 const &matrix, glm::vec3 const &vector) {
        glm::mat3 normMatrix(matrix);
        normMatrix = glm::transpose(glm::inverse(normMatrix));
        auto vector3 = normMatrix * vector;
        return glm::normalize(vector3);
    }

    virtual void apply() override {
        zeno::vec3f translate = {0,0,0};
        zeno::vec4f rotation = {0,0,0,1};
        zeno::vec3f eulerXYZ = {0,0,0};
        zeno::vec3f scaling = {1,1,1};
        zeno::vec3f shear = {0,0,0};
        zeno::vec3f offset = {0,0,0};
        glm::mat4 pre_mat = glm::mat4(1.0);
        glm::mat4 pre_apply = glm::mat4(1.0);
        glm::mat4 local = glm::mat4(1.0);
        if (has_input("Matrix"))
            pre_mat = std::get<glm::mat4>(get_input<zeno::MatrixObject>("Matrix")->m);
        if (has_input("translation"))
            translate = get_input<zeno::NumericObject>("translation")->get<zeno::vec3f>();
        if (has_input("eulerXYZ"))
            eulerXYZ = get_input<zeno::NumericObject>("eulerXYZ")->get<zeno::vec3f>();
        if (has_input("quatRotation"))
            rotation = get_input<zeno::NumericObject>("quatRotation")->get<zeno::vec4f>();
        if (has_input("scaling"))
            scaling = get_input<zeno::NumericObject>("scaling")->get<zeno::vec3f>();
        if (has_input("shear"))
            shear = get_input<zeno::NumericObject>("shear")->get<zeno::vec3f>();
        if (has_input("offset"))
            offset = get_input<zeno::NumericObject>("offset")->get<zeno::vec3f>();
        if (has_input("local"))
            local = std::get<glm::mat4>(get_input<zeno::MatrixObject>("local")->m);
        if (has_input("preTransform"))
            pre_apply = std::get<glm::mat4>(get_input<zeno::MatrixObject>("preTransform")->m);

        glm::mat4 matTrans = glm::translate(glm::vec3(translate[0], translate[1], translate[2]));
        glm::mat4 matRotx  = glm::rotate( (float)(eulerXYZ[0] * M_PI / 180), glm::vec3(1,0,0) );
        glm::mat4 matRoty  = glm::rotate( (float)(eulerXYZ[1] * M_PI / 180), glm::vec3(0,1,0) );
        glm::mat4 matRotz  = glm::rotate( (float)(eulerXYZ[2] * M_PI / 180), glm::vec3(0,0,1) );
        glm::quat myQuat(rotation[3], rotation[0], rotation[1], rotation[2]);
        glm::mat4 matQuat  = glm::toMat4(myQuat);
        glm::mat4 matScal  = glm::scale( glm::vec3(scaling[0], scaling[1], scaling[2] ));
        glm::mat4 matShearX = glm::transpose(glm::mat4(
                1, shear[0], 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1));
        glm::mat4 matShearY = glm::transpose(glm::mat4(
                1, 0, shear[1], 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1));
        glm::mat4 matShearZ = glm::transpose(glm::mat4(
                1, 0, 0, 0,
                0, 1, shear[2], 0,
                0, 0, 1, 0,
                0, 0, 0, 1));

        auto matrix = pre_mat*local*matTrans*matRoty*matRotx*matRotz*matQuat*matScal*matShearZ*matShearY*matShearX*glm::translate(glm::vec3(offset[0], offset[1], offset[2]))*glm::inverse(local)*pre_apply;

        auto prim = get_input<PrimitiveObject>("prim");

        std::string pivotType = get_input2<std::string>("pivot");
        if (pivotType == "bboxCenter") {
            zeno::vec3f _min;
            zeno::vec3f _max;
            objectGetBoundingBox(prim.get(), _min, _max);
            auto p = (_min + _max) / 2;
            auto pivot_to_local = glm::translate(glm::vec3(-p[0], -p[1], -p[2]));
            auto pivot_to_world = glm::translate(glm::vec3(p[0], p[1], p[2]));
            matrix = pivot_to_world * matrix * pivot_to_local;
        }

        auto outprim = std::make_unique<PrimitiveObject>(*prim);

        if (prim->has_attr("pos")) {
            auto &pos = outprim->attr<zeno::vec3f>("pos");
#pragma omp parallel for
            for (int i = 0; i < pos.size(); i++) {
                auto p = zeno::vec_to_other<glm::vec3>(pos[i]);
                p = mapplypos(matrix, p);
                pos[i] = zeno::other_to_vec<3>(p);
            }
        }

        if (prim->has_attr("nrm")) {
            auto &nrm = outprim->attr<zeno::vec3f>("nrm");
#pragma omp parallel for
            for (int i = 0; i < nrm.size(); i++) {
                auto n = zeno::vec_to_other<glm::vec3>(nrm[i]);
                n = mapplynrm(matrix, n);
                nrm[i] = zeno::other_to_vec<3>(n);
            }
        }

        auto& user_data = outprim->userData();
        user_data.setLiterial("_translate", translate);
        vec4f rotate = {myQuat.x, myQuat.y, myQuat.z, myQuat.w};
        user_data.setLiterial("_rotate", rotate);
        user_data.setLiterial("_scale", scaling);
        //auto oMat = std::make_shared<MatrixObject>();
        //oMat->m = matrix;
        set_output("outPrim", std::move(outprim));
    }
};

ZENDEFNODE(PrimitiveTransform, {
    {
        {"PrimitiveObject", "prim"},
        {"enum world bboxCenter", "pivot", "world"},
        {"vec3f", "translation", "0,0,0"},
        {"vec3f", "eulerXYZ", "0,0,0"},
        {"vec4f", "quatRotation", "0,0,0,1"},
        {"vec3f", "scaling", "1,1,1"},
        {"vec3f", "shear", "0,0,0"},
        {"Matrix"},
        {"preTransform"},
        {"local"},
    },
    {
        {"PrimitiveObject", "outPrim"}
    },
    {},
    {"primitive"},
});

}
}
