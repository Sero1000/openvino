// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//
#include <openvino/runtime/core.hpp>
#include <openvino/opsets/opset8.hpp>
#include <openvino/core/preprocess/pre_post_process.hpp>

void ppp_input_1(ov::preprocess::PrePostProcessor& ppp) {
//! [ov:preprocess:input_1]
ppp.input() // no index/name is needed if model has one input
  .preprocess().scale(50.f);

ppp.output()   // same for output
  .postprocess().convert_element_type(ov::element::u8);
//! [ov:preprocess:input_1]

//! [ov:preprocess:mean_scale]
ppp.input("input").preprocess().mean(128).scale(127);
//! [ov:preprocess:mean_scale]

//! [ov:preprocess:mean_scale_array]
// Suppose model's shape is {1, 3, 224, 224}
ppp.input("input").model().set_layout("NCHW"); // N=1, C=3, H=224, W=224
// Mean/Scale has 3 values which matches with C=3
ppp.input("input").preprocess()
  .mean({103.94, 116.78, 123.68}).scale({57.21, 57.45, 57.73});
//! [ov:preprocess:mean_scale_array]

//! [ov:preprocess:convert_element_type]
// First define data type for your tensor
ppp.input("input").tensor().set_element_type(ov::element::u8);

// Then define preprocessing step
ppp.input("input").preprocess().convert_element_type(ov::element::f32);

// If conversion is needed to `model's` element type, 'f32' can be omitted
ppp.input("input").preprocess().convert_element_type();
//! [ov:preprocess:convert_element_type]

//! [ov:preprocess:convert_layout]
// First define layout for your tensor
ppp.input("input").tensor().set_layout("NHWC");

// Then define layout of model
ppp.input("input").model().set_layout("NCHW");

std::cout << ppp; // Will print 'implicit layout conversion step'
//! [ov:preprocess:convert_layout]

//! [ov:preprocess:convert_layout_2]
ppp.input("input").tensor().set_shape({1, 480, 640, 3});
// Model expects shape {1, 3, 480, 640}
ppp.input("input").preprocess().convert_layout({0, 3, 1, 2});
// 0 -> 0; 3 -> 1; 1 -> 2; 2 -> 3
//! [ov:preprocess:convert_layout_2]

//! [ov:preprocess:resize_1]
ppp.input("input").tensor().set_shape({1, 3, 960, 1280});
ppp.input("input").model().set_layout("??HW");
ppp.input("input").preprocess().resize(ov::preprocess::ResizeAlgorithm::RESIZE_LINEAR, 480, 640);
//! [ov:preprocess:resize_1]

//! [ov:preprocess:resize_2]
ppp.input("input").tensor().set_shape({1, 3, 960, 1280});
ppp.input("input").model().set_layout("??HW"); // Model accepts {1, 3, 480, 640} shape
// Resize to model's dimension
ppp.input("input").preprocess().resize(ov::preprocess::ResizeAlgorithm::RESIZE_LINEAR);
//! [ov:preprocess:resize_2]

//! [ov:preprocess:convert_color_1]
ppp.input("input").tensor().set_color_format(ov::preprocess::ColorFormat::BGR);
ppp.input("input").preprocess().convert_color(ov::preprocess::ColorFormat::RGB);
//! [ov:preprocess:convert_color_1]

//! [ov:preprocess:convert_color_2]
// This will split original `input` to 2 separate inputs: `input/y' and 'input/uv'
ppp.input("input").tensor().set_color_format(ov::preprocess::ColorFormat::NV12_TWO_PLANES);
ppp.input("input").preprocess().convert_color(ov::preprocess::ColorFormat::RGB);
std::cout << ppp;  // Dump preprocessing steps to see what will happen
//! [ov:preprocess:convert_color_2]
}

void ppp_input_2(ov::preprocess::PrePostProcessor& ppp) {
 //! [ov:preprocess:input_index]
 auto &input_1 = ppp.input(1); // Gets 2nd input in a model
 auto &output_1 = ppp.output(2); // Get output with index=2 (3rd one) in a model
 //! [ov:preprocess:input_index]
}

void ppp_input_name(ov::preprocess::PrePostProcessor& ppp) {
 //! [ov:preprocess:input_name]
 auto &input_image = ppp.input("image");
 auto &output_result = ppp.output("result");
 //! [ov:preprocess:input_name]
}

int main() {
 std::string model_path;
 std::string input_name;
 //! [ov:preprocess:create]
 ov::Core core;
 std::shared_ptr<ov::Model> model = core.read_model(model_path);
 ov::preprocess::PrePostProcessor ppp(model);
 //! [ov:preprocess:create]

 //! [ov:preprocess:tensor]
 ov::preprocess::InputInfo& input = ppp.input(input_name);
 input.tensor()
   .set_element_type(ov::element::u8)
   .set_shape({1, 480, 640, 3})
   .set_layout("NHWC")
   .set_color_format(ov::preprocess::ColorFormat::BGR);
 //! [ov:preprocess:tensor]
 //! [ov:preprocess:model]
 // `model's input` already `knows` it's shape and data type, no need to specify them here
 input.model().set_layout("NCHW");
 //! [ov:preprocess:model]
 //! [ov:preprocess:steps]
 input.preprocess()
   .convert_element_type(ov::element::f32)
   .convert_color(ov::preprocess::ColorFormat::RGB)
   .resize(ov::preprocess::ResizeAlgorithm::RESIZE_LINEAR)
   .mean({100.5, 101, 101.5})
   .scale({50., 51., 52.});
   // Not needed, such conversion will be added implicitly
   // .convert_layout("NCHW");
 //! [ov:preprocess:steps]
 //! [ov:preprocess:custom]
 ppp.input("input_image").preprocess()
    .custom([](const ov::Output<ov::Node>& node) {
        // Custom nodes can be inserted as Pre-processing steps
        return std::make_shared<ov::opset8::Abs>(node);
    });
 //! [ov:preprocess:custom]
 //! [ov:preprocess:postprocess]
 // Model's output has 'NCHW' layout
 ppp.output("result_image").model().set_layout("NCHW");

 // Set target user's tensor to U8 type + 'NHWC' layout
 // Precision & layout conversions will be done implicitly
 ppp.output("result_image").tensor()
    .set_layout("NHWC")
    .set_element_type(ov::element::u8);

 // Also it is possible to insert some custom operations
 ppp.output("result_image").postprocess()
    .custom([](const ov::Output<ov::Node>& node) {
        // Custom nodes can be inserted as Post-processing steps
        return std::make_shared<ov::opset8::Abs>(node);
    });
 //! [ov:preprocess:postprocess]
 //! [ov:preprocess:build]
 std::cout << "Dump preprocessor: " << ppp << std::endl;
 model = ppp.build();
 //! [ov:preprocess:build]

 OPENVINO_ASSERT(model, "Model is invalid");
 return 0;
}
