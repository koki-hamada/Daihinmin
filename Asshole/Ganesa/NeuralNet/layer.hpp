#pragma once

class Layer {
public:
	int num_output_;
	int num_input_;
	int input_width_;
	int input_height_;
	int output_height_;
	int output_width_;

	Layer() {};
	Layer(	int num_output,
						int num_input,
						int input_width = 1,
						int input_height = 1,
						int output_height = 1,
						int output_width = 1) : 
						num_output_(num_output),
						num_input_(num_input),
						input_width_(input_width),
						input_height_(input_height),
						output_height_(output_height),
						output_width_(output_width) {};
	virtual ~Layer(){};
	virtual void Forward(const float *input, float* output) {};
};