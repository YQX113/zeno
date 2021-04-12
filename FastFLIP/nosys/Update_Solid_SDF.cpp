#include <zen/zen.h>
#include <zen/MeshObject.h>
#include <zen/VDBGrid.h>
#include <omp.h>
#include "FLIP_vdb.h"
//void FLIP_vdb::update_solid_sdf(std::vector<openvdb::FloatGrid::Ptr> &moving_solids, 
//openvdb::points::PointDataGrid::Ptr particles)

namespace zenbase{
    
    struct FLIP_Solid_Modifier : zen::INode{
        virtual void apply() override {
            auto particles = get_input("Particles")->as<VDBPointsGrid>();
            auto moving_solid_sdf = get_input("DynaSolid_SDF")->as<VDBFloatGrid>();
            auto static_solid_sdf = get_input("Static_SDF")->as<VDBFloatGrid>();
            std::vector<openvdb::FloatGrid::Ptr> moving_solids;
            moving_solids.emplace_back(moving_solid_sdf->m_grid);
            FLIP_vdb::update_solid_sdf(moving_solids, 
            static_solid_sdf->m_grid, particles->m_grid);
        }
    };

static int defFLIP_Solid_Modifier = zen::defNodeClass<FLIP_Solid_Modifier>("FLIP_Solid_Modifie",
    { /* inputs: */ {
        "Particles", "DynaSolid_SDF", "StatSolid_SDF",
    }, 
    /* outputs: */ {
    }, 
    /* params: */ {
      
    }, 
    
    /* category: */ {
    "FLIPSolver",
    }});

}