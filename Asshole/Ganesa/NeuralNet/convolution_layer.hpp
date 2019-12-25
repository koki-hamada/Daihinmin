#pragma once
#include "layer.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>

#ifdef USE_CBLAS
#include "cblas.h"
#endif

class ConvolutionLayer : public Layer {
  private:
	//float output_temp_[256][256];
	//float input_temp_[256][256];
	float* output_temp_;
	float* input_temp_;
	int w_, h_;

  public:
	float* weight_;
	float* bias_;
	int kernel_h_;
	int kernel_w_;
	int stride_;
	int pad_h_;
	int pad_w_;

	// �R���X�g���N�^
	ConvolutionLayer(int num_input, int input_height, int input_width, int num_output, int output_height, int output_width,
					 int kernel_height, int kernel_width, int stride, int pad_height, int pad_width)
		: Layer(num_output,
				num_input,
				input_width,
				input_height,
				output_height,
				output_width),
		  kernel_h_(kernel_height),
		  kernel_w_(kernel_width),
		  stride_(stride),
		  pad_h_(pad_height),
		  pad_w_(pad_width),
		  h_(input_height + pad_height * 2),
		  w_(input_width + pad_width * 2) {
		weight_ = new float[num_output * num_input * kernel_height * kernel_width];
		bias_ = new float[num_output];

#ifdef USE_CBLAS
		input_temp_ = new float[num_input * kernel_height * kernel_width * output_height * output_width];
#else
		input_temp_ = new float[h_ * w_];
		output_temp_ = new float[output_width * output_height];
#endif
	}

	// �f�X�g���N�^
	~ConvolutionLayer() {
		delete[] weight_;
		delete[] bias_;
		delete[] input_temp_;
		delete[] output_temp_;
	}

	bool is_a_ge_zero_and_a_lt_b(int a, int b) {
		return 0 <= a && a < b;
	}

	void im2col(const float* data_im, float* data_col) {
		const int input_h = input_height_;
		const int input_w = input_width_;
		const int output_h = output_height_;
		const int output_w = output_width_;
		assert(output_h == (input_h + 2 * pad_h_ - kernel_h_) / stride_ + 1);
		assert(output_w == (input_w + 2 * pad_w_ - kernel_w_) / stride_ + 1);

		const int channels = num_input_;
		const int channel_size = input_h * input_w;
		for(int channel = channels; channel--; data_im += channel_size) {
			for(int kernel_row = 0; kernel_row < kernel_h_; kernel_row++) {
				for(int kernel_col = 0; kernel_col < kernel_w_; kernel_col++) {
					int input_row = -pad_h_ + kernel_row;
					for(int output_rows = output_h; output_rows; output_rows--) {
						if(!is_a_ge_zero_and_a_lt_b(input_row, input_h)) {
							for(int output_cols = output_w; output_cols; output_cols--) {
								*(data_col++) = 0;
							}
						} else {
							int input_col = -pad_w_ + kernel_col;
							for(int output_col = output_w; output_col; output_col--) {
								if(is_a_ge_zero_and_a_lt_b(input_col, input_w)) {
									*(data_col++) = data_im[input_row * input_w + input_col];
								} else {
									*(data_col++) = 0;
								}
								input_col += stride_;
							}
						}
						input_row += stride_;
					}
				}
			}
		}
	}

	//���`�d.
	virtual void Forward(const float* input, float* output) {
#ifdef USE_CBLAS
		im2col(input, input_temp_);
		int M = num_output_;
		int N = output_height_ * output_width_;
		int K = num_input_ * kernel_h_ * kernel_w_;
		double alpha = 1.0;
		int lda = K;
		int ldb = N;
		int ldc = N;
		double beta = 0.0;
		cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, alpha, weight_, lda, input_temp_, ldb, beta, output, ldc);
		// bias
		for(int i = 0; i < num_output_ * output_height_ * output_width_; i++) {
			output[i] += bias_[i / (output_height_ * output_width_)];
		}
#else

		int index = 0;
		for(int m = 0; m < num_output_; m++) {
			for(int i = 0; i < output_height_ * output_width_; i++) {
				output_temp_[i] = 0.0f;
				//output[m*output_height_*output_width_ + y*output_width_ + x] = 0.0f;
			}
			int index_input = 0;
			for(int n = 0; n < num_input_; n++) {
				for(int i = 0; i < h_ * w_; i++)
					input_temp_[i] = 0.0f;
				for(int y = pad_h_; y < input_height_ + pad_h_; y++) {
					for(int x = pad_w_; x < input_width_ + pad_w_; x++, index_input++) {
						input_temp_[y * w_ + x] = input[index_input];
					}
				}
				for(int y = 0; y < output_height_; y++)
					for(int x = 0; x < output_width_; x++)
						for(int ky = 0; ky < kernel_h_; ky++)
							for(int kx = 0; kx < kernel_w_; kx++) {
								output_temp_[y * output_width_ + x] += input_temp_[(y * stride_ + ky) * w_ + x * stride_ + kx] * weight_[m * num_input_ * kernel_h_ * kernel_w_ + n * kernel_h_ * kernel_w_ + ky * kernel_w_ + kx];
								//output[m*output_height_*output_width_ + y*output_width_ + x] += input_temp_[y*stride_ + ky][x*stride_ + kx] * weight_[m * num_input_ * kernel_h_ * kernel_w_ + n * kernel_h_ * kernel_w_ + ky * kernel_w_ + kx];
							}
			}

			for(int i = 0; i < output_width_ * output_height_; i++) {
				output_temp_[i] += bias_[m];
				//output[m * output_height_ * output_width_ + y * output_width_ + x] += bias_[m];
			}
			for(int i = 0; i < output_width_ * output_height_; i++) {
				//assert((output[index] == output_temp_[y][x]));
				output[index++] = output_temp_[i];
			}
		}
#endif
	}
};
