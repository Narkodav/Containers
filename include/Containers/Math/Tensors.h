#pragma once
#include "Containers/Math/TensorDefinitions.h"
#include "Containers/Math/TensorFunctions.h"

namespace Containers::Math {
    static_assert(std::is_standard_layout_v<Tensor<float, 4>>);
    static_assert(std::is_trivially_copyable_v<Tensor<float, 4>>);

    static_assert(std::is_standard_layout_v<Tensor<float, 4, 4>>);
    static_assert(std::is_trivially_copyable_v<Tensor<float, 4, 4>>);

    // static_assert(sizeof(Tensor<float,4>) == 4 * sizeof(float));
    static_assert(alignof(Tensor<float,4>) == alignof(float));

    static_assert(Tensor<int, 4>::size() == 4);
    static_assert(Tensor<int, 2, 3>::size() == 6);
    static_assert(Tensor<int, 2, 3>::stride() == 3);
    static_assert(Tensor<int, 2, 3, 4>::stride() == 12);

    static_assert(std::same_as<
        Tensor<int, 2, 3>::ShapeType,
        Shape<int, 2, 3>
    >);

    // static_assert(sizeof(Tensor<int, 4>) == sizeof(int) * 4);
    // static_assert(sizeof(Tensor<float, 4, 4>) == sizeof(float) * 16);

    template<typename T, size_t size>
    using Vec = Tensor<T, size>;
    template<typename T>
    using Vec1 = Vec<T, 1>;
    template<typename T>
    using Vec2 = Vec<T, 2>;
    template<typename T>
    using Vec3 = Vec<T, 3>;
    template<typename T>
    using Vec4 = Vec<T, 4>;

    using IVec1 = Vec1<int32_t>;
    using IVec2 = Vec2<int32_t>;
    using IVec3 = Vec3<int32_t>;
    using IVec4 = Vec4<int32_t>;

    using UVec1 = Vec1<uint32_t>;
    using UVec2 = Vec2<uint32_t>;
    using UVec3 = Vec3<uint32_t>;
    using UVec4 = Vec4<uint32_t>;

    template<typename T, size_t row, size_t col>
    using Mat = Tensor<T, row, col>;
    template<typename T>
    using Mat1 = Mat<T, 1, 1>;
    template<typename T>
    using Mat2 = Mat<T, 2, 2>;
    template<typename T>
    using Mat3 = Mat<T, 3, 3>;
    template<typename T>
    using Mat4 = Mat<T, 4, 4>;

    using IMat1 = Mat1<int32_t>;
    using IMat2 = Mat2<int32_t>;
    using IMat3 = Mat3<int32_t>;
    using IMat4 = Mat4<int32_t>;

    using UMat1 = Mat1<uint32_t>;
    using UMat2 = Mat2<uint32_t>;
    using UMat3 = Mat3<uint32_t>;
    using UMat4 = Mat4<uint32_t>;

    static_assert(std::same_as<
        decltype(std::declval<IVec4>() + std::declval<IVec4>()),
        TensorBinaryExpression<
            IVec4,
            IVec4,
            BinaryOperationType::Add
        >
    >);

    static_assert(std::same_as<
        decltype(-std::declval<IVec4>()),
        TensorUnaryExpression<
            IVec4,
            UnaryOperationType::Negate
        >
    >);

    using Expr = decltype(
        std::declval<IVec4>() +
        std::declval<IVec4>() *
        std::declval<IVec4>()
    );

    static_assert(
        std::derived_from<
            Expr,
            TensorArithmeticExpression<Expr>
        >
    );
}