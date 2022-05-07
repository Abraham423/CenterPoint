/*
 * Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "argsParser.h"
#include "buffers.h"
#include "common.h"
#include "logger.h"
#include "parserOnnxConfig.h"
#include "NvInfer.h"
#include <cuda_runtime_api.h>
#include <cstdlib>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <chrono>
#include "preprocess.h"
#include "postprocess.h"
#include "scatter_cuda.h"
#include "centerpoint.h"
#include "utils.h"

const std::string gSampleName = "TensorRT.sample_onnx_centerpoint";





int main(int argc, char** argv)
{
    samplesCommon::Args args;
    bool argsOK = samplesCommon::parseArgs(args, argc, argv);
    if (!argsOK)
    {
        sample::gLogError << "Invalid arguments" << std::endl;
        return EXIT_FAILURE;
    }
    if (args.help)
    {
        return EXIT_SUCCESS;
    }
    auto sampleTest = sample::gLogger.defineTest(gSampleName, argc, argv);
    sample::gLogger.reportTestStart(sampleTest);



    ///////////////////////////////////////////////////////////////PARAM INITIALIZATION///////////////////////////////////////////////////////////////
    Params params;
    // initialize sample parameters 
    params.pfeOnnxFilePath = "../tools/pfe.onnx" ; // "../models/pfe_baseline32000.onnx";
    params.rpnOnnxFilePath = "../tools/rpn.onnx" ; //"../models/rpn_baseline.onnx";

    // params.pfeSerializedEnginePath = "/home/wanghao/Desktop/projects/notebooks/CP_ONNX_ENGINE/pfe_baseline_fp32.engine";
    // params.pfeSerializedEnginePath = "../models/pfe_baseline_fp16.engine";
    // params.rpnSerializedEnginePath = "../models/rpn_baseline_fp16.engine";

    params.pfeSerializedEnginePath = "../tools/pfe_fp.engine";
    params.rpnSerializedEnginePath = "../tools/rpn_fp.engine";

    // params.pfeSerializedEnginePath = "../tools/pfe_minmax10.engine";
    // params.rpnSerializedEnginePath = "../tools/rpn_minmax10.engine";

    // Input Output Names, according to TASK_NUM
    params.pfeInputTensorNames.push_back("input.1");
    params.rpnInputTensorNames.push_back("input.1");
    params.pfeOutputTensorNames.push_back("47");

    params.rpnOutputTensorNames["regName"]  = {"246"};
    params.rpnOutputTensorNames["rotName"] = {"258"};
    params.rpnOutputTensorNames["heightName"]={"250"};
    params.rpnOutputTensorNames["dimName"] = {"264"};
    params.rpnOutputTensorNames["scoreName"] = {"265"};
    params.rpnOutputTensorNames["clsName"] = {"266"};

    // Input Output Paths
    // params.savePath = "/home/wanghao/Desktop/projects/CenterPoint/tensorrt/data/centerpoint_pp_baseline_score0.1_nms0.7_gpuint8/" ;
    params.savePath = "../results" ;
    // params.filePaths=glob("../lidars/*.bin");
    params.filePaths=glob("/mnt/data/waymo_opensets/val/points/seq_201*.bin");

    // Attrs
    params.dlaCore = -1;
    params.fp16 = true;
    params.batch_size = 1;
    params.load_engine = true;
    // build int8 engine from file . 
    // params.load_engine +=  params.int8;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // std::string savePath = "/home/wanghao/Desktop/projects/notebooks/centerpoint_output_cpp" ;
    CenterPoint sample(params);
    sample::gLogInfo << "Building and running a GPU inference engine for CenterPoint" << std::endl;
    if (!sample.engineInitlization())
    {
        sample::gLogInfo << "sample build error  " << std::endl;
        return sample::gLogger.reportFail(sampleTest);
    }
    
    if (!sample.infer())
    {
        return sample::gLogger.reportFail(sampleTest);
    }

    sample::gLogger.reportPass(sampleTest);
    return 1;
}






