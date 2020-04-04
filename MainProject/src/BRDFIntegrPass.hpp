#pragma once
#include "RenderPass.hpp"

namespace renderer 
{

class BRDFIntegrPass : public Filter {
public:
    void OnInit() override;
    void Process() override;
};

};