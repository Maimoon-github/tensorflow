/* Copyright 2024 The OpenXLA Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <cstdint>

namespace stream_executor::redzone_checker_kernel {

namespace {
__global__ void kernel_func(uint8_t* input_buffer, uint8_t redzone_pattern,
                            uint64_t buffer_length,
                            uint32_t* out_mismatched_ptr) {
  uint64_t idx = threadIdx.x + blockIdx.x * blockDim.x;
  if (idx >= buffer_length) return;
  if (input_buffer[idx] != redzone_pattern) atomicAdd(out_mismatched_ptr, 1);
}
}  // namespace

void* kernel() { return reinterpret_cast<void*>(kernel_func); }

}  // namespace stream_executor::redzone_checker_kernel
