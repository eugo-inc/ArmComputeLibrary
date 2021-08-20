/*
 * Copyright (c) 2021 Arm Limited.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <cstddef>
#include <cstdint>

#if defined(ARM_COMPUTE_ENABLE_SVE)

namespace arm_conv {
namespace depthwise {

void sve_fp32_nhwc_3x3_s1_output4x4_mla_depthfirst_direct_impl(
  const unsigned int n_tile_rows,
  const unsigned int n_tile_cols,
  const float *inptr,
  int64_t ld_input_row,
  int64_t ld_input_col,
  float *outptr,
  int64_t ld_output_row,
  int64_t ld_output_col,
  const void *params,
  unsigned int n_channels,
  const float activation_min,
  const float activation_max
)
{
  struct Args
  {
    const uint64_t n_tile_rows, n_tile_cols;
    const float *inptr;
    const uint64_t ld_input_row;
    const uint64_t ld_input_col;
    float *outptr;
    const uint64_t ld_output_row;
    const uint64_t ld_output_col;
    const void *params;
    const float min, max;

    uint64_t tile_i = 0, tile_j = 0;

    Args(
      const unsigned int n_tile_rows,
      const unsigned int n_tile_cols,
      const float *inptr,
      int64_t ld_input_row,
      int64_t ld_input_col,
      float *outptr,
      int64_t ld_output_row,
      int64_t ld_output_col,
      const void *params,
      const float activation_min,
      const float activation_max
    ) : n_tile_rows(n_tile_rows), n_tile_cols(n_tile_cols), inptr(inptr),
        ld_input_row(ld_input_row), ld_input_col(ld_input_col), outptr(outptr),
        ld_output_row(ld_output_row), ld_output_col(ld_output_col),
        params(params), min(activation_min), max(activation_max)
    {
    }
  };

  Args params_struct(
    n_tile_rows, n_tile_cols,
    inptr, ld_input_row, ld_input_col,
    outptr, ld_output_row, ld_output_col,
    params, activation_min, activation_max
  );

  __asm__ __volatile__(
    "ptrue p3.b\n"
    "mov x2, #0x0\n"
    "mov x3, #0x0\n"
    "1:"  // Tile loop
    "str x2, [%x[params_struct], %[offsetof_args_tile_i]]\n"
    "mov x24, #0x4\n"
    "str x3, [%x[params_struct], %[offsetof_args_tile_j]]\n"
    "mov x23, #0x4\n"
    "ldr x4, [%x[params_struct], %[offsetof_args_params]]\n"
    "mov x5, #0x0\n"
    "ldr x22, [%x[params_struct], %[offsetof_args_ld_input_row]]\n"
    "cntw x6\n"
    "ldr x7, [%x[params_struct], %[offsetof_args_ld_input_col]]\n"
    "sub x21, XZR, x6\n"
    "ldr x8, [%x[params_struct], %[offsetof_args_inptr]]\n"
    "mul x19, x2, x22\n" // offset = tile_i * ld_input_row
    "ldr x20, [%x[params_struct], %[offsetof_args_ld_output_row]]\n"
    "madd x19, x3, x7, x19\n" // offset += tile_j * ld_input_col
    "ldr x17, [%x[params_struct], %[offsetof_args_ld_output_col]]\n"
    "mul x19, x19, x24\n" // offset *= kernel_stride * output_size
    "ldr x16, [%x[params_struct], %[offsetof_args_outptr]]\n"
    "add x8, x8, x19, LSL #2\n" // inptr[0] += offset * sizeof(float)
    "ld1rw { z15.s }, p3/Z, [%x[params_struct], %[offsetof_args_min]]\n"
    "add x15, x8, x22, LSL #2\n"
    "ld1rw { z14.s }, p3/Z, [%x[params_struct], %[offsetof_args_max]]\n"
    "add x14, x15, x22, LSL #2\n"
    "ld1w { z13.s }, p3/Z, [x4]\n"
    "mov z31.d, z13.d\n"
    "ld1w { z0.s }, p3/Z, [x4, #1, MUL VL]\n"
    "add x13, x14, x22, LSL #2\n"
    "mov z30.d, z13.d\n"
    "ld1w { z1.s }, p3/Z, [x4, #2, MUL VL]\n"
    "add x12, x13, x22, LSL #2\n"
    "mov z29.d, z13.d\n"
    "ld1w { z2.s }, p3/Z, [x4, #3, MUL VL]\n"
    "add x11, x12, x22, LSL #2\n"
    "mov z28.d, z13.d\n"
    "ld1w { z3.s }, p3/Z, [x4, #4, MUL VL]\n"
    "add x10, x7, x7\n"
    "mov z27.d, z13.d\n"
    "ld1w { z4.s }, p3/Z, [x4, #5, MUL VL]\n"
    "add x9, x10, x7\n"
    "mov z26.d, z13.d\n"
    "ld1w { z5.s }, p3/Z, [x4, #6, MUL VL]\n"
    "add x28, x9, x7\n"
    "mov z25.d, z13.d\n"
    "ld1w { z6.s }, p3/Z, [x4, #7, MUL VL]\n"
    "add x27, x28, x7\n"
    "mov z24.d, z13.d\n"
    "mul x19, x2, x20\n" // offset = tile_i * ld_output_row
    "mov z23.d, z13.d\n"
    "madd x19, x3, x17, x19\n" // offset += tile_j * ld_output_col
    "mov z22.d, z13.d\n"
    "mul x19, x19, x23\n" // offset *= output_tile_size
    "mov z21.d, z13.d\n"
    "add x16, x16, x19, LSL #2\n" // outptrs[0] += offset * sizeof(float)
    "mov z20.d, z13.d\n"
    "add x26, x16, x20, LSL #2\n"
    "mov z19.d, z13.d\n"
    "add x25, x26, x20, LSL #2\n"
    "mov z18.d, z13.d\n"
    "add x24, x25, x20, LSL #2\n"
    "mov z17.d, z13.d\n"
    "add x23, x17, x17\n"
    "mov z16.d, z13.d\n"
    "add x22, x23, x17\n"
    "whilelt p2.s, XZR, %x[n_channels]\n"
    "ld1w { z9.s }, p2/Z, [x14, x10, LSL #2]\n"
    "ld1w { z10.s }, p2/Z, [x8]\n"
    "addvl x4, x4, #16\n"
    "ld1w { z11.s }, p2/Z, [x8, x27, LSL #2]\n"
    "cmp x6, %x[n_channels]\n"
    "ld1w { z7.s }, p3/Z, [x4, #-8, MUL VL]\n"
    "ld1w { z8.s }, p3/Z, [x4, #-7, MUL VL]\n"
    "addvl x4, x4, #-6\n"
    "ld1w { z12.s }, p2/Z, [x14, x9, LSL #2]\n"
    "bge 3f\n"
    "2:"  // Tile loop: Channel loop
    "fmla z31.s, p3/M, z8.s, z9.s\n"
    "ld1w { z13.s }, p3/Z, [x4]\n"
    "whilelt p1.s, x6, %x[n_channels]\n"
    "fmla z30.s, p3/M, z7.s, z9.s\n"
    "incw x21\n"
    "fmla z29.s, p3/M, z6.s, z9.s\n"
    "mov p0.b, p2.b\n"
    "fmla z27.s, p3/M, z5.s, z9.s\n"
    "incw x5\n"
    "fmla z26.s, p3/M, z4.s, z9.s\n"
    "incw x6\n"
    "fmla z25.s, p3/M, z3.s, z9.s\n"
    "fmla z23.s, p3/M, z2.s, z9.s\n"
    "fmla z22.s, p3/M, z1.s, z9.s\n"
    "fmla z21.s, p3/M, z0.s, z9.s\n"
    "ld1w { z9.s }, p2/Z, [x13, x10, LSL #2]\n"
    "fmla z31.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x11]\n"
    "fmla z28.s, p3/M, z2.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x27, LSL #2]\n"
    "fmla z30.s, p3/M, z8.s, z12.s\n"
    "fmla z29.s, p3/M, z7.s, z12.s\n"
    "fmla z26.s, p3/M, z5.s, z12.s\n"
    "fmla z28.s, p3/M, z6.s, z12.s\n"
    "fmla z25.s, p3/M, z4.s, z12.s\n"
    "fmla z24.s, p3/M, z3.s, z12.s\n"
    "fmla z22.s, p3/M, z2.s, z12.s\n"
    "fmla z21.s, p3/M, z1.s, z12.s\n"
    "fmla z20.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x8, x7, LSL #2]\n"
    "fmla z19.s, p3/M, z6.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x13, x9, LSL #2]\n"
    "fmla z16.s, p3/M, z8.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x8, x28, LSL #2]\n"
    "fmla z27.s, p3/M, z8.s, z9.s\n"
    "fmla z26.s, p3/M, z7.s, z9.s\n"
    "fmla z25.s, p3/M, z6.s, z9.s\n"
    "fmla z23.s, p3/M, z5.s, z9.s\n"
    "fmla z22.s, p3/M, z4.s, z9.s\n"
    "fmla z21.s, p3/M, z3.s, z9.s\n"
    "fmla z19.s, p3/M, z2.s, z9.s\n"
    "fmla z18.s, p3/M, z1.s, z9.s\n"
    "fmla z17.s, p3/M, z0.s, z9.s\n"
    "ld1w { z9.s }, p2/Z, [x15]\n"
    "fmla z31.s, p3/M, z1.s, z12.s\n"
    "fmla z30.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x15, x27, LSL #2]\n"
    "fmla z29.s, p3/M, z2.s, z11.s\n"
    "fmla z28.s, p3/M, z1.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12]\n"
    "fmla z26.s, p3/M, z8.s, z10.s\n"
    "fmla z25.s, p3/M, z7.s, z10.s\n"
    "fmla z24.s, p3/M, z6.s, z10.s\n"
    "fmla z22.s, p3/M, z5.s, z10.s\n"
    "fmla z21.s, p3/M, z4.s, z10.s\n"
    "fmla z20.s, p3/M, z3.s, z10.s\n"
    "fmla z18.s, p3/M, z2.s, z10.s\n"
    "fmla z17.s, p3/M, z1.s, z10.s\n"
    "fmla z16.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x15, x10, LSL #2]\n"
    "fmla z31.s, p3/M, z3.s, z9.s\n"
    "fmla z27.s, p3/M, z0.s, z9.s\n"
    "fmla z28.s, p3/M, z5.s, z12.s\n"
    "fmla z24.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x15, x9, LSL #2]\n"
    "fmla z23.s, p3/M, z6.s, z11.s\n"
    "fmla z19.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x27, LSL #2]\n"
    "fmla z31.s, p3/M, z5.s, z10.s\n"
    "fmla z30.s, p3/M, z4.s, z10.s\n"
    "fmla z29.s, p3/M, z3.s, z10.s\n"
    "fmla z27.s, p3/M, z2.s, z10.s\n"
    "fmla z26.s, p3/M, z1.s, z10.s\n"
    "fmla z25.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x14, x7, LSL #2]\n"
    "fmla z20.s, p3/M, z8.s, z11.s\n"
    "fmla z16.s, p3/M, z5.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x7, LSL #2]\n"
    "fmla z30.s, p3/M, z5.s, z12.s\n"
    "fmla z29.s, p3/M, z4.s, z12.s\n"
    "fmla z28.s, p3/M, z3.s, z12.s\n"
    "fmla z26.s, p3/M, z2.s, z12.s\n"
    "fmla z25.s, p3/M, z1.s, z12.s\n"
    "fmla z24.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x14, x28, LSL #2]\n"
    "fmla z19.s, p3/M, z7.s, z11.s\n"
    "fmla z18.s, p3/M, z6.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x28, LSL #2]\n"
    "fmla z31.s, p3/M, z7.s, z10.s\n"
    "fmla z30.s, p3/M, z6.s, z10.s\n"
    "fmla z27.s, p3/M, z4.s, z10.s\n"
    "fmla z26.s, p3/M, z3.s, z10.s\n"
    "fmla z23.s, p3/M, z1.s, z10.s\n"
    "fmla z22.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x8, x10, LSL #2]\n"
    "fmla z17.s, p3/M, z8.s, z11.s\n"
    "fmla z16.s, p3/M, z7.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x13, x7, LSL #2]\n"
    "fmla z29.s, p3/M, z8.s, z12.s\n"
    "fmla z28.s, p3/M, z7.s, z12.s\n"
    "fmla z25.s, p3/M, z5.s, z12.s\n"
    "fmla z24.s, p3/M, z4.s, z12.s\n"
    "fmla z21.s, p3/M, z2.s, z12.s\n"
    "fmla z20.s, p3/M, z1.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x8, x9, LSL #2]\n"
    "addvl x8, x8, #1\n"
    "fmla z31.s, p3/M, z2.s, z10.s\n"
    "fmla z30.s, p3/M, z1.s, z10.s\n"
    "fmla z29.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x14]\n"
    "fmla z27.s, p3/M, z7.s, z11.s\n"
    "fmla z26.s, p3/M, z6.s, z11.s\n"
    "fmla z23.s, p3/M, z4.s, z11.s\n"
    "fmla z22.s, p3/M, z3.s, z11.s\n"
    "fmla z19.s, p3/M, z1.s, z11.s\n"
    "fmla z18.s, p3/M, z0.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x13, x28, LSL #2]\n"
    "fmla z30.s, p3/M, z2.s, z12.s\n"
    "fmla z29.s, p3/M, z1.s, z12.s\n"
    "fmla z28.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x14, x27, LSL #2]\n"
    "addvl x14, x14, #1\n"
    "fmla z31.s, p3/M, z6.s, z10.s\n"
    "ld1w { z9.s }, p1/Z, [x14, x10, LSL #2]\n"
    "fmla z27.s, p3/M, z3.s, z10.s\n"
    "fmla z23.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x13]\n"
    "fmla z25.s, p3/M, z8.s, z11.s\n"
    "fmla z24.s, p3/M, z7.s, z11.s\n"
    "fmla z21.s, p3/M, z5.s, z11.s\n"
    "fmla z20.s, p3/M, z4.s, z11.s\n"
    "fmla z17.s, p3/M, z2.s, z11.s\n"
    "fmla z16.s, p3/M, z1.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x10, LSL #2]\n"
    "fmla z28.s, p3/M, z8.s, z12.s\n"
    "fmla z24.s, p3/M, z5.s, z12.s\n"
    "fmla z20.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x13, x27, LSL #2]\n"
    "addvl x13, x13, #1\n"
    "fmla z27.s, p3/M, z6.s, z10.s\n"
    "fmla z23.s, p3/M, z3.s, z10.s\n"
    "fmla z19.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x11, x10, LSL #2]\n"
    "fmla z22.s, p3/M, z7.s, z11.s\n"
    "fmla z21.s, p3/M, z6.s, z11.s\n"
    "fmla z23.s, p3/M, z8.s, z11.s\n"
    "fmla z19.s, p3/M, z5.s, z11.s\n"
    "fmla z18.s, p3/M, z4.s, z11.s\n"
    "fmla z17.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x9, LSL #2]\n"
    "fmla z24.s, p3/M, z8.s, z12.s\n"
    "fmla z20.s, p3/M, z5.s, z12.s\n"
    "fmla z16.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x11, x9, LSL #2]\n"
    "addvl x11, x11, #1\n"
    "fmla z19.s, p3/M, z8.s, z10.s\n"
    "fmla z18.s, p3/M, z7.s, z10.s\n"
    "fmla z17.s, p3/M, z6.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x15, x7, LSL #2]\n"
    "fmla z22.s, p3/M, z8.s, z11.s\n"
    "fmla z21.s, p3/M, z7.s, z11.s\n"
    "fmla z20.s, p3/M, z6.s, z11.s\n"
    "fmla z18.s, p3/M, z5.s, z11.s\n"
    "fmla z17.s, p3/M, z4.s, z11.s\n"
    "fmla z16.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x15, x28, LSL #2]\n"
    "addvl x15, x15, #1\n"
    "fmla z18.s, p3/M, z8.s, z12.s\n"
    "fmla z31.s, p3/M, z4.s, z10.s\n"
    "fmla z17.s, p3/M, z7.s, z12.s\n"
    "fmla z16.s, p3/M, z6.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x12, x7, LSL #2]\n"
    "fmla z30.s, p3/M, z3.s, z10.s\n"
    "fmla z27.s, p3/M, z1.s, z10.s\n"
    "fmla z26.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x12, x28, LSL #2]\n"
    "whilelt p2.s, x5, %x[n_channels]\n"
    "fmla z29.s, p3/M, z5.s, z11.s\n"
    "ld1w { z0.s }, p3/Z, [x4, #1, MUL VL]\n"
    "addvl x12, x12, #1\n"
    "fmla z28.s, p3/M, z4.s, z11.s\n"
    "cmp x6, %x[n_channels]\n"
    "fmla z25.s, p3/M, z2.s, z11.s\n"
    "ld1w { z2.s }, p3/Z, [x4, #3, MUL VL]\n"
    "fmla z24.s, p3/M, z1.s, z11.s\n"
    "ld1w { z11.s }, p1/Z, [x8, x27, LSL #2]\n"
    "fmla z23.s, p3/M, z7.s, z12.s\n"
    "ld1w { z1.s }, p3/Z, [x4, #2, MUL VL]\n"
    "fmla z22.s, p3/M, z6.s, z12.s\n"
    "ld1w { z6.s }, p3/Z, [x4, #7, MUL VL]\n"
    "fmla z19.s, p3/M, z4.s, z12.s\n"
    "fmla z18.s, p3/M, z3.s, z12.s\n"
    "ld1w { z12.s }, p1/Z, [x14, x9, LSL #2]\n"
    "fmla z21.s, p3/M, z8.s, z10.s\n"
    "ld1w { z3.s }, p3/Z, [x4, #4, MUL VL]\n"
    "fmla z20.s, p3/M, z7.s, z10.s\n"
    "fmla z17.s, p3/M, z5.s, z10.s\n"
    "ld1w { z5.s }, p3/Z, [x4, #6, MUL VL]\n"
    "fmla z16.s, p3/M, z4.s, z10.s\n"
    "ld1w { z10.s }, p1/Z, [x8]\n"
    "fmax z31.s, p3/M, z31.s, z15.s\n"
    "ld1w { z4.s }, p3/Z, [x4, #5, MUL VL]\n"
    "addvl x4, x4, #16\n"
    "fmax z30.s, p3/M, z30.s, z15.s\n"
    "ld1w { z7.s }, p3/Z, [x4, #-8, MUL VL]\n"
    "fmax z29.s, p3/M, z29.s, z15.s\n"
    "ld1w { z8.s }, p3/Z, [x4, #-7, MUL VL]\n"
    "addvl x4, x4, #-6\n"
    "fmin z31.s, p3/M, z31.s, z14.s\n"
    "st1w { z31.s }, p0, [x16]\n"
    "mov z31.d, z13.d\n"
    "fmin z30.s, p3/M, z30.s, z14.s\n"
    "st1w { z30.s }, p0, [x16, x17, LSL #2]\n"
    "mov z30.d, z13.d\n"
    "fmin z29.s, p3/M, z29.s, z14.s\n"
    "st1w { z29.s }, p0, [x16, x23, LSL #2]\n"
    "mov z29.d, z13.d\n"
    "fmax z28.s, p3/M, z28.s, z15.s\n"
    "fmax z27.s, p3/M, z27.s, z15.s\n"
    "fmax z26.s, p3/M, z26.s, z15.s\n"
    "fmax z25.s, p3/M, z25.s, z15.s\n"
    "fmin z28.s, p3/M, z28.s, z14.s\n"
    "st1w { z28.s }, p0, [x16, x22, LSL #2]\n"
    "mov z28.d, z13.d\n"
    "addvl x16, x16, #1\n"
    "fmin z27.s, p3/M, z27.s, z14.s\n"
    "st1w { z27.s }, p0, [x26]\n"
    "mov z27.d, z13.d\n"
    "fmin z26.s, p3/M, z26.s, z14.s\n"
    "st1w { z26.s }, p0, [x26, x17, LSL #2]\n"
    "mov z26.d, z13.d\n"
    "fmin z25.s, p3/M, z25.s, z14.s\n"
    "st1w { z25.s }, p0, [x26, x23, LSL #2]\n"
    "mov z25.d, z13.d\n"
    "fmax z24.s, p3/M, z24.s, z15.s\n"
    "fmax z23.s, p3/M, z23.s, z15.s\n"
    "fmax z22.s, p3/M, z22.s, z15.s\n"
    "fmax z21.s, p3/M, z21.s, z15.s\n"
    "fmin z24.s, p3/M, z24.s, z14.s\n"
    "st1w { z24.s }, p0, [x26, x22, LSL #2]\n"
    "mov z24.d, z13.d\n"
    "addvl x26, x26, #1\n"
    "fmin z23.s, p3/M, z23.s, z14.s\n"
    "st1w { z23.s }, p0, [x25]\n"
    "mov z23.d, z13.d\n"
    "fmin z22.s, p3/M, z22.s, z14.s\n"
    "st1w { z22.s }, p0, [x25, x17, LSL #2]\n"
    "mov z22.d, z13.d\n"
    "fmin z21.s, p3/M, z21.s, z14.s\n"
    "st1w { z21.s }, p0, [x25, x23, LSL #2]\n"
    "mov z21.d, z13.d\n"
    "fmax z20.s, p3/M, z20.s, z15.s\n"
    "fmax z19.s, p3/M, z19.s, z15.s\n"
    "fmax z18.s, p3/M, z18.s, z15.s\n"
    "fmax z17.s, p3/M, z17.s, z15.s\n"
    "fmin z20.s, p3/M, z20.s, z14.s\n"
    "st1w { z20.s }, p0, [x25, x22, LSL #2]\n"
    "mov z20.d, z13.d\n"
    "addvl x25, x25, #1\n"
    "fmin z19.s, p3/M, z19.s, z14.s\n"
    "st1w { z19.s }, p0, [x24]\n"
    "mov z19.d, z13.d\n"
    "fmin z18.s, p3/M, z18.s, z14.s\n"
    "st1w { z18.s }, p0, [x24, x17, LSL #2]\n"
    "mov z18.d, z13.d\n"
    "fmin z17.s, p3/M, z17.s, z14.s\n"
    "st1w { z17.s }, p0, [x24, x23, LSL #2]\n"
    "mov z17.d, z13.d\n"
    "fmax z16.s, p3/M, z16.s, z15.s\n"
    "fmin z16.s, p3/M, z16.s, z14.s\n"
    "st1w { z16.s }, p0, [x24, x22, LSL #2]\n"
    "mov z16.d, z13.d\n"
    "addvl x24, x24, #1\n"
    "blt 2b\n"
    "3:"  // Tile loop: Channel tail
    "fmla z31.s, p3/M, z8.s, z9.s\n"
    "ldr x2, [%x[params_struct], %[offsetof_args_tile_i]]\n"
    "mov p0.b, p2.b\n"
    "fmla z30.s, p3/M, z7.s, z9.s\n"
    "ldr x3, [%x[params_struct], %[offsetof_args_tile_j]]\n"
    "add x21, x2, #0x1\n"
    "fmla z29.s, p3/M, z6.s, z9.s\n"
    "ldr x20, [%x[params_struct], %[offsetof_args_n_tile_rows]]\n"
    "fmla z27.s, p3/M, z5.s, z9.s\n"
    "ldr x19, [%x[params_struct], %[offsetof_args_n_tile_cols]]\n"
    "add x3, x3, #0x1\n"
    "fmla z26.s, p3/M, z4.s, z9.s\n"
    "cmp x3, x19\n"
    "fmla z25.s, p3/M, z3.s, z9.s\n"
    "fmla z23.s, p3/M, z2.s, z9.s\n"
    "csel x3, x3, XZR, LT\n"
    "fmla z22.s, p3/M, z1.s, z9.s\n"
    "csel x2, x2, x21, LT\n"
    "fmla z21.s, p3/M, z0.s, z9.s\n"
    "ld1w { z9.s }, p2/Z, [x13, x10, LSL #2]\n"
    "cmp x2, x20\n"
    "fmla z31.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x11]\n"
    "fmla z28.s, p3/M, z2.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x27, LSL #2]\n"
    "fmla z30.s, p3/M, z8.s, z12.s\n"
    "fmla z29.s, p3/M, z7.s, z12.s\n"
    "fmla z26.s, p3/M, z5.s, z12.s\n"
    "fmla z28.s, p3/M, z6.s, z12.s\n"
    "fmla z25.s, p3/M, z4.s, z12.s\n"
    "fmla z24.s, p3/M, z3.s, z12.s\n"
    "fmla z22.s, p3/M, z2.s, z12.s\n"
    "fmla z21.s, p3/M, z1.s, z12.s\n"
    "fmla z20.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x8, x7, LSL #2]\n"
    "fmla z19.s, p3/M, z6.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x13, x9, LSL #2]\n"
    "fmla z16.s, p3/M, z8.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x8, x28, LSL #2]\n"
    "fmla z27.s, p3/M, z8.s, z9.s\n"
    "fmla z26.s, p3/M, z7.s, z9.s\n"
    "fmla z25.s, p3/M, z6.s, z9.s\n"
    "fmla z23.s, p3/M, z5.s, z9.s\n"
    "fmla z22.s, p3/M, z4.s, z9.s\n"
    "fmla z21.s, p3/M, z3.s, z9.s\n"
    "fmla z19.s, p3/M, z2.s, z9.s\n"
    "fmla z18.s, p3/M, z1.s, z9.s\n"
    "fmla z17.s, p3/M, z0.s, z9.s\n"
    "ld1w { z9.s }, p2/Z, [x15]\n"
    "fmla z31.s, p3/M, z1.s, z12.s\n"
    "fmla z30.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x15, x27, LSL #2]\n"
    "fmla z29.s, p3/M, z2.s, z11.s\n"
    "fmla z28.s, p3/M, z1.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12]\n"
    "fmla z26.s, p3/M, z8.s, z10.s\n"
    "fmla z25.s, p3/M, z7.s, z10.s\n"
    "fmla z24.s, p3/M, z6.s, z10.s\n"
    "fmla z22.s, p3/M, z5.s, z10.s\n"
    "fmla z21.s, p3/M, z4.s, z10.s\n"
    "fmla z20.s, p3/M, z3.s, z10.s\n"
    "fmla z18.s, p3/M, z2.s, z10.s\n"
    "fmla z17.s, p3/M, z1.s, z10.s\n"
    "fmla z16.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x15, x10, LSL #2]\n"
    "fmla z31.s, p3/M, z3.s, z9.s\n"
    "fmla z27.s, p3/M, z0.s, z9.s\n"
    "fmla z28.s, p3/M, z5.s, z12.s\n"
    "fmla z24.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x15, x9, LSL #2]\n"
    "fmla z23.s, p3/M, z6.s, z11.s\n"
    "fmla z19.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x27, LSL #2]\n"
    "fmla z31.s, p3/M, z5.s, z10.s\n"
    "fmla z30.s, p3/M, z4.s, z10.s\n"
    "fmla z29.s, p3/M, z3.s, z10.s\n"
    "fmla z27.s, p3/M, z2.s, z10.s\n"
    "fmla z26.s, p3/M, z1.s, z10.s\n"
    "fmla z25.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x14, x7, LSL #2]\n"
    "fmla z20.s, p3/M, z8.s, z11.s\n"
    "fmla z16.s, p3/M, z5.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x7, LSL #2]\n"
    "fmla z30.s, p3/M, z5.s, z12.s\n"
    "fmla z29.s, p3/M, z4.s, z12.s\n"
    "fmla z28.s, p3/M, z3.s, z12.s\n"
    "fmla z26.s, p3/M, z2.s, z12.s\n"
    "fmla z25.s, p3/M, z1.s, z12.s\n"
    "fmla z24.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x14, x28, LSL #2]\n"
    "fmla z19.s, p3/M, z7.s, z11.s\n"
    "fmla z18.s, p3/M, z6.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x11, x28, LSL #2]\n"
    "fmla z31.s, p3/M, z7.s, z10.s\n"
    "fmla z30.s, p3/M, z6.s, z10.s\n"
    "fmla z27.s, p3/M, z4.s, z10.s\n"
    "fmla z26.s, p3/M, z3.s, z10.s\n"
    "fmla z23.s, p3/M, z1.s, z10.s\n"
    "fmla z22.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x8, x10, LSL #2]\n"
    "fmla z17.s, p3/M, z8.s, z11.s\n"
    "fmla z16.s, p3/M, z7.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x13, x7, LSL #2]\n"
    "fmla z29.s, p3/M, z8.s, z12.s\n"
    "fmla z28.s, p3/M, z7.s, z12.s\n"
    "fmla z25.s, p3/M, z5.s, z12.s\n"
    "fmla z24.s, p3/M, z4.s, z12.s\n"
    "fmla z21.s, p3/M, z2.s, z12.s\n"
    "fmla z20.s, p3/M, z1.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x8, x9, LSL #2]\n"
    "fmla z31.s, p3/M, z2.s, z10.s\n"
    "fmla z30.s, p3/M, z1.s, z10.s\n"
    "fmla z29.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x14]\n"
    "fmla z27.s, p3/M, z7.s, z11.s\n"
    "fmla z26.s, p3/M, z6.s, z11.s\n"
    "fmla z23.s, p3/M, z4.s, z11.s\n"
    "fmla z22.s, p3/M, z3.s, z11.s\n"
    "fmla z19.s, p3/M, z1.s, z11.s\n"
    "fmla z18.s, p3/M, z0.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x13, x28, LSL #2]\n"
    "fmla z30.s, p3/M, z2.s, z12.s\n"
    "fmla z29.s, p3/M, z1.s, z12.s\n"
    "fmla z28.s, p3/M, z0.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x14, x27, LSL #2]\n"
    "fmla z31.s, p3/M, z6.s, z10.s\n"
    "fmla z27.s, p3/M, z3.s, z10.s\n"
    "fmla z23.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x13]\n"
    "fmla z25.s, p3/M, z8.s, z11.s\n"
    "fmla z24.s, p3/M, z7.s, z11.s\n"
    "fmla z21.s, p3/M, z5.s, z11.s\n"
    "fmla z20.s, p3/M, z4.s, z11.s\n"
    "fmla z17.s, p3/M, z2.s, z11.s\n"
    "fmla z16.s, p3/M, z1.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x10, LSL #2]\n"
    "fmla z28.s, p3/M, z8.s, z12.s\n"
    "fmla z24.s, p3/M, z5.s, z12.s\n"
    "fmla z20.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x13, x27, LSL #2]\n"
    "fmla z27.s, p3/M, z6.s, z10.s\n"
    "fmla z23.s, p3/M, z3.s, z10.s\n"
    "fmla z19.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x11, x10, LSL #2]\n"
    "fmla z22.s, p3/M, z7.s, z11.s\n"
    "fmla z21.s, p3/M, z6.s, z11.s\n"
    "fmla z23.s, p3/M, z8.s, z11.s\n"
    "fmla z19.s, p3/M, z5.s, z11.s\n"
    "fmla z18.s, p3/M, z4.s, z11.s\n"
    "fmla z17.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x12, x9, LSL #2]\n"
    "fmla z24.s, p3/M, z8.s, z12.s\n"
    "fmla z20.s, p3/M, z5.s, z12.s\n"
    "fmla z16.s, p3/M, z2.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x11, x9, LSL #2]\n"
    "fmla z19.s, p3/M, z8.s, z10.s\n"
    "fmla z18.s, p3/M, z7.s, z10.s\n"
    "fmla z17.s, p3/M, z6.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x15, x7, LSL #2]\n"
    "fmla z22.s, p3/M, z8.s, z11.s\n"
    "fmla z21.s, p3/M, z7.s, z11.s\n"
    "fmla z20.s, p3/M, z6.s, z11.s\n"
    "fmla z18.s, p3/M, z5.s, z11.s\n"
    "fmla z17.s, p3/M, z4.s, z11.s\n"
    "fmla z16.s, p3/M, z3.s, z11.s\n"
    "ld1w { z11.s }, p2/Z, [x15, x28, LSL #2]\n"
    "fmla z31.s, p3/M, z4.s, z10.s\n"
    "fmla z18.s, p3/M, z8.s, z12.s\n"
    "fmla z17.s, p3/M, z7.s, z12.s\n"
    "fmla z16.s, p3/M, z6.s, z12.s\n"
    "ld1w { z12.s }, p2/Z, [x12, x7, LSL #2]\n"
    "fmla z30.s, p3/M, z3.s, z10.s\n"
    "fmla z27.s, p3/M, z1.s, z10.s\n"
    "fmla z26.s, p3/M, z0.s, z10.s\n"
    "ld1w { z10.s }, p2/Z, [x12, x28, LSL #2]\n"
    "fmla z29.s, p3/M, z5.s, z11.s\n"
    "fmla z28.s, p3/M, z4.s, z11.s\n"
    "fmla z25.s, p3/M, z2.s, z11.s\n"
    "fmla z24.s, p3/M, z1.s, z11.s\n"
    "fmla z23.s, p3/M, z7.s, z12.s\n"
    "fmla z22.s, p3/M, z6.s, z12.s\n"
    "fmla z19.s, p3/M, z4.s, z12.s\n"
    "fmla z18.s, p3/M, z3.s, z12.s\n"
    "fmla z21.s, p3/M, z8.s, z10.s\n"
    "fmla z20.s, p3/M, z7.s, z10.s\n"
    "fmla z17.s, p3/M, z5.s, z10.s\n"
    "fmla z16.s, p3/M, z4.s, z10.s\n"
    "fmax z31.s, p3/M, z31.s, z15.s\n"
    "fmax z30.s, p3/M, z30.s, z15.s\n"
    "fmax z29.s, p3/M, z29.s, z15.s\n"
    "fmax z28.s, p3/M, z28.s, z15.s\n"
    "fmin z31.s, p3/M, z31.s, z14.s\n"
    "st1w { z31.s }, p0, [x16]\n"
    "fmin z30.s, p3/M, z30.s, z14.s\n"
    "fmin z29.s, p3/M, z29.s, z14.s\n"
    "st1w { z30.s }, p0, [x16, x17, LSL #2]\n"
    "fmin z28.s, p3/M, z28.s, z14.s\n"
    "fmax z27.s, p3/M, z27.s, z15.s\n"
    "st1w { z29.s }, p0, [x16, x23, LSL #2]\n"
    "fmax z26.s, p3/M, z26.s, z15.s\n"
    "st1w { z28.s }, p0, [x16, x22, LSL #2]\n"
    "fmin z27.s, p3/M, z27.s, z14.s\n"
    "fmax z25.s, p3/M, z25.s, z15.s\n"
    "st1w { z27.s }, p0, [x26]\n"
    "fmin z26.s, p3/M, z26.s, z14.s\n"
    "fmin z25.s, p3/M, z25.s, z14.s\n"
    "st1w { z26.s }, p0, [x26, x17, LSL #2]\n"
    "fmax z24.s, p3/M, z24.s, z15.s\n"
    "fmax z23.s, p3/M, z23.s, z15.s\n"
    "st1w { z25.s }, p0, [x26, x23, LSL #2]\n"
    "fmax z22.s, p3/M, z22.s, z15.s\n"
    "fmax z21.s, p3/M, z21.s, z15.s\n"
    "fmax z20.s, p3/M, z20.s, z15.s\n"
    "fmin z24.s, p3/M, z24.s, z14.s\n"
    "st1w { z24.s }, p0, [x26, x22, LSL #2]\n"
    "fmin z23.s, p3/M, z23.s, z14.s\n"
    "fmin z22.s, p3/M, z22.s, z14.s\n"
    "st1w { z23.s }, p0, [x25]\n"
    "fmin z21.s, p3/M, z21.s, z14.s\n"
    "fmin z20.s, p3/M, z20.s, z14.s\n"
    "st1w { z22.s }, p0, [x25, x17, LSL #2]\n"
    "fmax z19.s, p3/M, z19.s, z15.s\n"
    "st1w { z21.s }, p0, [x25, x23, LSL #2]\n"
    "fmax z18.s, p3/M, z18.s, z15.s\n"
    "fmax z17.s, p3/M, z17.s, z15.s\n"
    "st1w { z20.s }, p0, [x25, x22, LSL #2]\n"
    "fmin z19.s, p3/M, z19.s, z14.s\n"
    "st1w { z19.s }, p0, [x24]\n"
    "fmin z18.s, p3/M, z18.s, z14.s\n"
    "fmin z17.s, p3/M, z17.s, z14.s\n"
    "st1w { z18.s }, p0, [x24, x17, LSL #2]\n"
    "fmax z16.s, p3/M, z16.s, z15.s\n"
    "st1w { z17.s }, p0, [x24, x23, LSL #2]\n"
    "fmin z16.s, p3/M, z16.s, z14.s\n"
    "st1w { z16.s }, p0, [x24, x22, LSL #2]\n"
    "blt 1b\n"
    :
    : [n_channels] "r" ((unsigned long) n_channels), [offsetof_args_inptr] "I" (offsetof(Args, inptr)), [offsetof_args_ld_input_col] "I" (offsetof(Args, ld_input_col)), [offsetof_args_ld_input_row] "I" (offsetof(Args, ld_input_row)), [offsetof_args_ld_output_col] "I" (offsetof(Args, ld_output_col)), [offsetof_args_ld_output_row] "I" (offsetof(Args, ld_output_row)), [offsetof_args_max] "I" (offsetof(Args, max)), [offsetof_args_min] "I" (offsetof(Args, min)), [offsetof_args_n_tile_cols] "I" (offsetof(Args, n_tile_cols)), [offsetof_args_n_tile_rows] "I" (offsetof(Args, n_tile_rows)), [offsetof_args_outptr] "I" (offsetof(Args, outptr)), [offsetof_args_params] "I" (offsetof(Args, params)), [offsetof_args_tile_i] "I" (offsetof(Args, tile_i)), [offsetof_args_tile_j] "I" (offsetof(Args, tile_j)), [params_struct] "r" (&params_struct)
    : "cc", "memory", "p0", "p1", "p2", "p3", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "z0", "z1", "z2", "z3", "z4", "z5", "z6", "z7", "z8", "z9", "z10", "z11", "z12", "z13", "z14", "z15", "z16", "z17", "z18", "z19", "z20", "z21", "z22", "z23", "z24", "z25", "z26", "z27", "z28", "z29", "z30", "z31"
  );
}

}  // namespace depthwise
}  // namespace arm_conv

#endif  // defined(ARM_COMPUTE_ENABLE_SVE)