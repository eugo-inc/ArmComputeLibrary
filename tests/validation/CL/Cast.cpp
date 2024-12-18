/*
 * Copyright (c) 2018-2020, 2022-2024 Arm Limited.
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
#include "arm_compute/core/Types.h"
#include "arm_compute/runtime/CL/CLTensor.h"
#include "arm_compute/runtime/CL/CLTensorAllocator.h"
#include "arm_compute/runtime/CL/functions/CLCast.h"
#include "tests/CL/CLAccessor.h"
#include "tests/datasets/ConvertPolicyDataset.h"
#include "tests/datasets/DatatypeDataset.h"
#include "tests/datasets/ShapeDatasets.h"
#include "tests/framework/Asserts.h"
#include "tests/framework/Macros.h"
#include "tests/framework/datasets/Datasets.h"
#include "tests/validation/Validation.h"
#include "tests/validation/fixtures/CastFixture.h"

#include <map>
#include <vector>

namespace arm_compute
{
namespace test
{
namespace validation
{

using framework::dataset::make;
namespace
{
// Tolerance
constexpr AbsoluteTolerance<float> one_tolerance(1);
constexpr AbsoluteTolerance<float> zero_tolerance(0);

/** Input data sets **/
// QASYMM8
const auto CastQASYMM8toF32Dataset = combine(make("DataType", DataType::QASYMM8), make("DataType", DataType::F32));
const auto CastQSYMM8toF32Dataset = combine(make("DataType", DataType::QSYMM8), make("DataType", DataType::F32));

#define U8Types DataType::U8, DataType::QASYMM8
#define S8Types DataType::S8, DataType::QSYMM8, DataType::QSYMM8_PER_CHANNEL, DataType::QASYMM8_SIGNED
#define S8Types_wo_q8_pc DataType::S8, DataType::QSYMM8, DataType::QASYMM8_SIGNED
#define U16Types DataType::QASYMM16, DataType::U16
#define S16Types DataType::S16, DataType::QSYMM16

// U8
const auto CastU8toS8Dataset  = combine(make("DataType", {U8Types}), make("DataType", {S8Types}));
const auto CastU8toU16Dataset = combine(make("DataType", {U8Types}), make("DataType", {U16Types}));
const auto CastU8toS16Dataset = combine(make("DataType", {U8Types}), make("DataType", {S16Types}));
const auto CastU8toU32Dataset = combine(make("DataType", {U8Types}), make("DataType", DataType::U32));
const auto CastU8toS32Dataset = combine(make("DataType", {U8Types}), make("DataType", DataType::S32));
const auto CastU8toF16Dataset = combine(make("DataType", {U8Types}), make("DataType", DataType::F16));
const auto CastU8toF32Dataset = combine(make("DataType", {U8Types}), make("DataType", DataType::F32));

// S8
const auto CastS8toU8Dataset  = combine(make("DataType", {S8Types_wo_q8_pc}), make("DataType", {U8Types}));
const auto CastQSYMM8_PER_CHANNELtoU8Dataset  = combine(make("DataType", DataType::QSYMM8_PER_CHANNEL), make("DataType", DataType::U8));
const auto CastS8toU16Dataset = combine(make("DataType", {S8Types}), make("DataType", {U16Types}));
const auto CastS8toS16Dataset = combine(make("DataType", {S8Types}), make("DataType", {S16Types}));
const auto CastS8toU32Dataset = combine(make("DataType", {S8Types}), make("DataType", DataType::U32));
const auto CastS8toS32Dataset = combine(make("DataType", {S8Types}), make("DataType", DataType::S32));
const auto CastS8toF16Dataset = combine(make("DataType", {S8Types}), make("DataType", DataType::F16));
const auto CastS8toF32Dataset = combine(make("DataType", {S8Types}), make("DataType", DataType::F32));

// U16
const auto CastU16toU8Dataset  = combine(make("DataType", {U16Types}), make("DataType", {U8Types}));
const auto CastU16toS8Dataset  = combine(make("DataType", {U16Types}), make("DataType", {S8Types}));
const auto CastU16toS16Dataset = combine(make("DataType", {U16Types}), make("DataType", {S16Types}));
const auto CastU16toU32Dataset = combine(make("DataType", {U16Types}), make("DataType", DataType::U32));
const auto CastU16toS32Dataset = combine(make("DataType", {U16Types}), make("DataType", DataType::S32));
const auto CastU16toF16Dataset = combine(make("DataType", {U16Types}), make("DataType", DataType::F16));
const auto CastU16toF32Dataset = combine(make("DataType", {U16Types}), make("DataType", DataType::F32));

// S16
const auto CastS16toU8Dataset  = combine(make("DataType", {S16Types}), make("DataType", {U8Types}));
const auto CastS16toS8Dataset  = combine(make("DataType", {S16Types}), make("DataType", {S8Types}));
const auto CastS16toU16Dataset = combine(make("DataType", {S16Types}), make("DataType", {U16Types}));
const auto CastS16toU32Dataset = combine(make("DataType", {S16Types}), make("DataType", DataType::U32));
const auto CastS16toS32Dataset = combine(make("DataType", {S16Types}), make("DataType", DataType::S32));
const auto CastS16toF16Dataset = combine(make("DataType", {S16Types}), make("DataType", DataType::F16));
const auto CastS16toF32Dataset = combine(make("DataType", {S16Types}), make("DataType", DataType::F32));

// U32
const auto CastU32toU8Dataset  = combine(make("DataType", DataType::U32), make("DataType", {U8Types}));
const auto CastU32toS8Dataset  = combine(make("DataType", DataType::U32), make("DataType", {S8Types}));
const auto CastU32toU16Dataset = combine(make("DataType", DataType::U32), make("DataType", {U16Types}));
const auto CastU32toS16Dataset = combine(make("DataType", DataType::U32), make("DataType", {S16Types}));
const auto CastU32toS32Dataset = combine(make("DataType", DataType::U32), make("DataType", DataType::S32));
const auto CastU32toF16Dataset = combine(make("DataType", DataType::U32), make("DataType", DataType::F16));
const auto CastU32toF32Dataset = combine(make("DataType", DataType::U32), make("DataType", DataType::F32));

// S32
const auto CastS32toU8Dataset  = combine(make("DataType", DataType::S32), make("DataType", {U8Types}));
const auto CastS32toS8Dataset  = combine(make("DataType", DataType::S32), make("DataType", {S8Types}));
const auto CastS32toU16Dataset = combine(make("DataType", DataType::S32), make("DataType", {U16Types}));
const auto CastS32toS16Dataset = combine(make("DataType", DataType::S32), make("DataType", {S16Types}));
const auto CastS32toU32Dataset = combine(make("DataType", DataType::S32), make("DataType", DataType::U32));
const auto CastS32toF16Dataset = combine(make("DataType", DataType::S32), make("DataType", DataType::F16));
const auto CastS32toF32Dataset = combine(make("DataType", DataType::S32), make("DataType", DataType::F32));

// F16
const auto CastF16toU8Dataset  = combine(make("DataType", DataType::F16), make("DataType", {U8Types}));
const auto CastF16toS8Dataset  = combine(make("DataType", DataType::F16), make("DataType", {S8Types}));
const auto CastF16toU16Dataset = combine(make("DataType", DataType::F16), make("DataType", {U16Types}));
const auto CastF16toS16Dataset = combine(make("DataType", DataType::F16), make("DataType", {S16Types}));
const auto CastF16toU32Dataset = combine(make("DataType", DataType::F16), make("DataType", DataType::U32));
const auto CastF16toS32Dataset = combine(make("DataType", DataType::F16), make("DataType", DataType::S32));
const auto CastF16toF32Dataset = combine(make("DataType", DataType::F16), make("DataType", DataType::F32));

// F32
const auto CastF32toU8Dataset  = combine(make("DataType", DataType::F32), make("DataType", {U8Types}));
const auto CastF32toS8Dataset  = combine(make("DataType", DataType::F32), make("DataType", {S8Types}));
const auto CastF32toU16Dataset = combine(make("DataType", DataType::F32), make("DataType", {U16Types}));
const auto CastF32toS16Dataset = combine(make("DataType", DataType::F32), make("DataType", {S16Types}));
const auto CastF32toU32Dataset = combine(make("DataType", DataType::F32), make("DataType", DataType::U32));
const auto CastF32toS32Dataset = combine(make("DataType", DataType::F32), make("DataType", DataType::S32));
const auto CastF32toF16Dataset = combine(make("DataType", DataType::F32), make("DataType", DataType::F16));

// U64
const auto CastU64toU8Dataset  = combine(make("DataType", DataType::U64), make("DataType", {U8Types}));
const auto CastU64toS8Dataset  = combine(make("DataType", DataType::U64), make("DataType", {S8Types}));
const auto CastU64toU16Dataset = combine(make("DataType", DataType::U64), make("DataType", {U16Types}));
const auto CastU64toS16Dataset = combine(make("DataType", DataType::U64), make("DataType", {S16Types}));
const auto CastU64toU32Dataset = combine(make("DataType", DataType::U64), make("DataType", DataType::U32));
const auto CastU64toS32Dataset = combine(make("DataType", DataType::U64), make("DataType", DataType::S32));
const auto CastU64toF16Dataset = combine(make("DataType", DataType::U64), make("DataType", DataType::F16));
const auto CastU64toF32Dataset = combine(make("DataType", DataType::U64), make("DataType", DataType::F32));

// S64
const auto CastS64toU8Dataset  = combine(make("DataType", DataType::S64), make("DataType", {U8Types}));
const auto CastS64toS8Dataset  = combine(make("DataType", DataType::S64), make("DataType", {S8Types}));
const auto CastS64toU16Dataset = combine(make("DataType", DataType::S64), make("DataType", {U16Types}));
const auto CastS64toS16Dataset = combine(make("DataType", DataType::S64), make("DataType", {S16Types}));
const auto CastS64toU32Dataset = combine(make("DataType", DataType::S64), make("DataType", DataType::U32));
const auto CastS64toS32Dataset = combine(make("DataType", DataType::S64), make("DataType", DataType::S32));
const auto CastS64toF16Dataset = combine(make("DataType", DataType::S64), make("DataType", DataType::F16));
const auto CastS64toF32Dataset = combine(make("DataType", DataType::S64), make("DataType", DataType::F32));

void validate_data_types(DataType input_dtype, DataType output_dtype)
{
    const auto input = TensorInfo(TensorShape(16U, 16U, 5U), 1, input_dtype);
    auto output = TensorInfo(TensorShape(16U, 16U, 5U), 1, output_dtype);

    const Status status = (CLCast::validate(&input, &output, ConvertPolicy::SATURATE));
    const bool is_valid = static_cast<bool>(status);

    static std::map<DataType, std::vector<DataType>> supported_dtypes;

    supported_dtypes[DataType::U8] = {
        S8Types, U16Types, S16Types, DataType::U32, DataType::S32, DataType::F16, DataType::F32};

    supported_dtypes[DataType::S8] = {
        U8Types, U16Types, S16Types, DataType::U32, DataType::S32, DataType::F16, DataType::F32};

    supported_dtypes[DataType::U16] = {
        DataType::U8, DataType::S8, DataType::S16, DataType::U32, DataType::S32, DataType::F16, DataType::F32,
        DataType::QSYMM8, DataType::QASYMM8, DataType::QSYMM8_PER_CHANNEL, DataType::QASYMM8_SIGNED, DataType::QSYMM16};

    supported_dtypes[DataType::S16] = {
        S8Types, U8Types, U16Types, DataType::U32, DataType::S32, DataType::F16, DataType::F32};

    supported_dtypes[DataType::U32] = {
        S8Types, U8Types, U16Types, S16Types, DataType::S32, DataType::F16, DataType::F32};

    supported_dtypes[DataType::S32] = {
        S8Types, U8Types, U16Types, S16Types, DataType::U32, DataType::F16, DataType::F32};

    supported_dtypes[DataType::U64] = {
        S8Types, U8Types, U16Types, S16Types, DataType::U32, DataType::S32,
        DataType::F16, DataType::F32};

    supported_dtypes[DataType::S64] = {
        S8Types, U8Types, U16Types, S16Types, DataType::U32, DataType::S32,
        DataType::F16, DataType::F32};

    supported_dtypes[DataType::F16] = {
        S8Types, U8Types, U16Types, S16Types, DataType::U32, DataType::S32, DataType::F32};

    supported_dtypes[DataType::F32] = {
       S8Types, U8Types, U16Types, S16Types, DataType::U32, DataType::S32, DataType::F16};

    supported_dtypes[DataType::QSYMM8] = supported_dtypes[DataType::S8];
    supported_dtypes[DataType::QASYMM8_SIGNED] = supported_dtypes[DataType::S8];
    supported_dtypes[DataType::QSYMM8_PER_CHANNEL] = {
        U16Types, S16Types, DataType::U8, DataType::U32, DataType::S32, DataType::F16, DataType::F32
    };

    supported_dtypes[DataType::QASYMM8] = supported_dtypes[DataType::U8];

    supported_dtypes[DataType::QSYMM16] = supported_dtypes[DataType::S16];
    supported_dtypes[DataType::QASYMM16] = supported_dtypes[DataType::U16];

    bool expected = false;
    if(supported_dtypes.find(input_dtype) != supported_dtypes.end())
    {
        const auto supports = supported_dtypes[input_dtype];
        expected = (std::find(supports.begin(), supports.end(), output_dtype) != supports.end());
    }

    ARM_COMPUTE_EXPECT_EQUAL(is_valid, expected, framework::LogLevel::ERRORS);

    if(is_valid != expected)
    {
        std::cout << status.error_description() << std::endl;
    }
}
} // namespace

TEST_SUITE(CL)
TEST_SUITE(Cast)
template <typename T>
using CLCastToU8Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, uint8_t>;
template <typename T>
using CLCastToS8Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, int8_t>;
template <typename T>
using CLCastToU16Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, uint16_t>;
template <typename T>
using CLCastToS16Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, int16_t>;
template <typename T>
using CLCastToU32Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, uint32_t>;
template <typename T>
using CLCastToS32Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, int32_t>;
template <typename T>
using CLCastToF16Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, half>;
template <typename T>
using CLCastToF32Fixture = CastValidationFixture<CLTensor, CLAccessor, CLCast, T, float>;

DATA_TEST_CASE(ValidateAllDataTypes, framework::DatasetMode::ALL,
    combine(
        datasets::AllDataTypes("InputDataType"),
        datasets::AllDataTypes("OutputDataType")),
        input_dtype, output_dtype)
{
    validate_data_types(input_dtype, output_dtype);
}


#define CAST_SUITE(NAME, type, dataset, tolerance)                                                                     \
    TEST_SUITE(NAME)                                                                                                             \
    FIXTURE_DATA_TEST_CASE(RunSmall, type, framework::DatasetMode::PRECOMMIT, combine(combine(datasets::SmallShapes(), dataset), \
                                                                                      datasets::ConvertPolicies()))              \
    {                                                                                                                            \
        validate(CLAccessor(_target), _reference, tolerance);                                                                    \
    }                                                                                                                            \
    TEST_SUITE_END()

// U8
CAST_SUITE(U8_to_S8, CLCastToS8Fixture<uint8_t>, CastU8toS8Dataset, zero_tolerance)
CAST_SUITE(U8_to_U16, CLCastToU16Fixture<uint8_t>, CastU8toU16Dataset, zero_tolerance)
CAST_SUITE(U8_to_S16, CLCastToS16Fixture<uint8_t>, CastU8toS16Dataset, zero_tolerance)
CAST_SUITE(U8_to_U32, CLCastToU32Fixture<uint8_t>, CastU8toU32Dataset, zero_tolerance)
CAST_SUITE(U8_to_S32,  CLCastToS32Fixture<uint8_t>, CastU8toS32Dataset, zero_tolerance)
CAST_SUITE(U8_to_F16, CLCastToF16Fixture<uint8_t>, CastU8toF16Dataset, zero_tolerance)
CAST_SUITE(U8_to_F32, CLCastToF32Fixture<uint8_t>, CastU8toF32Dataset, zero_tolerance)

// S8
CAST_SUITE(S8_to_U8, CLCastToU8Fixture<int8_t>, CastS8toU8Dataset, zero_tolerance)
CAST_SUITE(S8_to_U16, CLCastToU16Fixture<int8_t>, CastS8toU16Dataset, zero_tolerance)
CAST_SUITE(S8_to_S16, CLCastToS16Fixture<int8_t>, CastS8toS16Dataset, zero_tolerance)
CAST_SUITE(S8_to_U32, CLCastToU32Fixture<int8_t>, CastS8toU32Dataset, zero_tolerance)
CAST_SUITE(S8_to_S32, CLCastToS32Fixture<int8_t>, CastS8toS32Dataset, zero_tolerance)
CAST_SUITE(S8_to_F16, CLCastToF16Fixture<int8_t>, CastS8toF16Dataset, zero_tolerance)
CAST_SUITE(S8_to_F32, CLCastToF32Fixture<int8_t>, CastS8toF32Dataset, zero_tolerance)

// U16
CAST_SUITE(U16_to_U8, CLCastToU8Fixture<uint16_t>, CastU16toU8Dataset, zero_tolerance)
CAST_SUITE(U16_to_S8, CLCastToS8Fixture<uint16_t>, CastU16toS8Dataset, zero_tolerance)
CAST_SUITE(U16_to_S16, CLCastToS16Fixture<uint16_t>, CastU16toS16Dataset, zero_tolerance)
CAST_SUITE(U16_to_U32, CLCastToU32Fixture<uint16_t>, CastU16toU32Dataset, zero_tolerance)
CAST_SUITE(U16_to_S32, CLCastToS32Fixture<uint16_t>, CastU16toS32Dataset, zero_tolerance)
CAST_SUITE(U16_to_F16, CLCastToF16Fixture<uint16_t>, CastU16toF16Dataset, zero_tolerance)
CAST_SUITE(U16_to_F32, CLCastToF32Fixture<uint16_t>, CastU16toF32Dataset, zero_tolerance)

// S16
CAST_SUITE(S16_to_U8, CLCastToU8Fixture<int16_t>, CastS16toU8Dataset, zero_tolerance)
CAST_SUITE(S16_to_S8, CLCastToS8Fixture<int16_t>, CastS16toS8Dataset, zero_tolerance)
CAST_SUITE(S16_to_U16, CLCastToU16Fixture<int16_t>, CastS16toU16Dataset, zero_tolerance)
CAST_SUITE(S16_to_U32, CLCastToU32Fixture<int16_t>, CastS16toU32Dataset, zero_tolerance)
CAST_SUITE(S16_to_S32, CLCastToS32Fixture<int16_t>, CastS16toS32Dataset, zero_tolerance)
CAST_SUITE(S16_to_F16, CLCastToF16Fixture<int16_t>, CastS16toF16Dataset, zero_tolerance)
CAST_SUITE(S16_to_F32, CLCastToF32Fixture<int16_t>, CastS16toF32Dataset, zero_tolerance)

// U32
CAST_SUITE(U32_to_U8, CLCastToU8Fixture<uint32_t>, CastU32toU8Dataset, zero_tolerance)
CAST_SUITE(U32_to_S8, CLCastToS8Fixture<uint32_t>, CastU32toS8Dataset, zero_tolerance)
CAST_SUITE(U32_to_U16, CLCastToU16Fixture<uint32_t>, CastU32toU16Dataset, zero_tolerance)
CAST_SUITE(U32_to_S16, CLCastToS16Fixture<uint32_t>, CastU32toS16Dataset, zero_tolerance)
CAST_SUITE(U32_to_S32, CLCastToS32Fixture<uint32_t>, CastU32toS32Dataset, zero_tolerance)
CAST_SUITE(U32_to_F16, CLCastToF16Fixture<uint32_t>, CastU32toF16Dataset, zero_tolerance)
CAST_SUITE(U32_to_F32, CLCastToF32Fixture<uint32_t>, CastU32toF32Dataset, zero_tolerance)

// S32
CAST_SUITE(S32_to_U8, CLCastToU8Fixture<int32_t>, CastS32toU8Dataset, zero_tolerance)
CAST_SUITE(S32_to_S8, CLCastToS8Fixture<int32_t>, CastS32toS8Dataset, zero_tolerance)
CAST_SUITE(S32_to_U16, CLCastToU16Fixture<int32_t>, CastS32toU16Dataset, zero_tolerance)
CAST_SUITE(S32_to_S16, CLCastToS16Fixture<int32_t>, CastS32toS16Dataset, zero_tolerance)
CAST_SUITE(S32_to_U32, CLCastToU32Fixture<int32_t>, CastS32toU32Dataset, zero_tolerance)
CAST_SUITE(S32_to_F16, CLCastToF16Fixture<int32_t>, CastS32toF16Dataset, zero_tolerance)
CAST_SUITE(S32_to_F32, CLCastToF32Fixture<int32_t>, CastS32toF32Dataset, zero_tolerance)

// F16
CAST_SUITE(F16_to_U8, CLCastToU8Fixture<half>, CastF16toU8Dataset, one_tolerance)
CAST_SUITE(F16_to_S8, CLCastToS8Fixture<half>, CastF16toS8Dataset, one_tolerance)
CAST_SUITE(F16_to_U16, CLCastToU16Fixture<half>, CastF16toU16Dataset, one_tolerance)
CAST_SUITE(F16_to_S16, CLCastToS16Fixture<half>, CastF16toS16Dataset, one_tolerance)
CAST_SUITE(F16_to_U32, CLCastToU32Fixture<half>, CastF16toU32Dataset, one_tolerance)
CAST_SUITE(F16_to_S32, CLCastToS32Fixture<half>, CastF16toS32Dataset, one_tolerance)
CAST_SUITE(F16_to_F32, CLCastToF32Fixture<half>, CastF16toF32Dataset, zero_tolerance)

// F32
CAST_SUITE(F32_to_U8, CLCastToU8Fixture<float>, CastF32toU8Dataset, one_tolerance)
CAST_SUITE(F32_to_S8, CLCastToS8Fixture<float>, CastF32toS8Dataset, one_tolerance)
CAST_SUITE(F32_to_U16, CLCastToU16Fixture<float>, CastF32toU16Dataset, one_tolerance)
CAST_SUITE(F32_to_S16, CLCastToS16Fixture<float>, CastF32toS16Dataset, one_tolerance)
CAST_SUITE(F32_to_U32, CLCastToU32Fixture<float>, CastF32toU32Dataset, one_tolerance)
CAST_SUITE(F32_to_S32, CLCastToS32Fixture<float>, CastF32toS32Dataset, one_tolerance)
CAST_SUITE(F32_to_F16, CLCastToF16Fixture<float>, CastF32toF16Dataset, zero_tolerance)

// S64
CAST_SUITE(S64_to_U8, CLCastToU8Fixture<int64_t>, CastS64toU8Dataset, one_tolerance)
CAST_SUITE(S64_to_S8, CLCastToS8Fixture<int64_t>, CastS64toS8Dataset, one_tolerance)
CAST_SUITE(S64_to_U16, CLCastToU16Fixture<int64_t>, CastS64toU16Dataset, one_tolerance)
CAST_SUITE(S64_to_S16, CLCastToS16Fixture<int64_t>, CastS64toS16Dataset, one_tolerance)
CAST_SUITE(S64_to_U32, CLCastToU32Fixture<int64_t>, CastS64toU32Dataset, one_tolerance)
CAST_SUITE(S64_to_S32, CLCastToS32Fixture<int64_t>, CastS64toS32Dataset, one_tolerance)
CAST_SUITE(S64_to_F16, CLCastToF16Fixture<int64_t>, CastS64toF16Dataset, zero_tolerance)
CAST_SUITE(S64_to_F32, CLCastToF32Fixture<int64_t>, CastS64toF32Dataset, zero_tolerance)

// U64
CAST_SUITE(U64_to_U8, CLCastToU8Fixture<uint64_t>, CastU64toU8Dataset, one_tolerance)
CAST_SUITE(U64_to_S8, CLCastToS8Fixture<uint64_t>, CastU64toS8Dataset, one_tolerance)
CAST_SUITE(U64_to_U16, CLCastToU16Fixture<uint64_t>, CastU64toU16Dataset, one_tolerance)
CAST_SUITE(U64_to_S16, CLCastToS16Fixture<uint64_t>, CastU64toS16Dataset, one_tolerance)
CAST_SUITE(U64_to_U32, CLCastToU32Fixture<uint64_t>, CastU64toU32Dataset, one_tolerance)
CAST_SUITE(U64_to_S32, CLCastToS32Fixture<uint64_t>, CastU64toS32Dataset, one_tolerance)
CAST_SUITE(U64_to_F16, CLCastToF16Fixture<uint64_t>, CastU64toF16Dataset, zero_tolerance)
CAST_SUITE(U64_to_F32, CLCastToF32Fixture<uint64_t>, CastU64toF32Dataset, zero_tolerance)

TEST_SUITE_END() // Cast
TEST_SUITE_END() // CL
} // namespace validation
} // namespace test
} // namespace arm_compute
