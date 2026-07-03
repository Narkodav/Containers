#pragma once
#include <stdexcept>
#include <cassert>
#include <concepts>
#include <array>

#include "Containers/Math/Tensors.h"

namespace Containers::Math {

    template<typename T, size_t... dims>
    struct Shape;

    template<typename T, size_t N, size_t... rest>
    struct Shape<T, N, rest...> {
        using ValueType = T;
        static constexpr size_t s_stride = (rest * ...);
        static constexpr size_t s_size = N * s_stride;
        static constexpr size_t s_dimCount = 1 + sizeof...(rest);
        static constexpr std::array<size_t, s_dimCount> s_dims = { N, rest... };

        static constexpr std::array<size_t, s_dimCount> dims() { return s_dims; }
        static constexpr size_t dimCount() { return s_dimCount; }
        static constexpr size_t size() { return s_size; }
        static constexpr size_t stride() { return s_stride; }

        template<typename I, typename...Rest>
        static constexpr size_t index(I&& index, Rest&&... restIs) {
            return index * s_stride + Shape<T, rest...>::index(std::forward<Rest>(restIs)...);
        }
    };

    template<typename T, size_t N>
    struct Shape<T, N> {
        using ValueType = T;
        static constexpr size_t s_stride = 1;
        static constexpr size_t s_size = N;
        static constexpr size_t s_dimCount = 1;
        static constexpr std::array<size_t, 1> s_dims = { N };

        static constexpr std::array<size_t, 1> dims() { return s_dims; }
        static constexpr size_t dimCount() { return s_dimCount; }
        static constexpr size_t size() { return s_size; }
        static constexpr size_t stride() { return s_stride; }

        static constexpr size_t index(size_t index) {
            return index;
        }
    };

    template<typename D>
    class TensorExpression {
    public:
        using Derived = D;
        using NotIncludedInScalarArithmetic = void;

        constexpr const Derived& derived() const {
            return static_cast<const Derived&>(*this);
        }

        static constexpr size_t size() {
            return Derived::size();
        }

        constexpr auto eval(size_t i) const {
            return derived().eval(i);
        }

        constexpr auto eval() const {
            return derived().eval();
        }
    };

    template<typename Derived>
    class TensorArithmeticExpression : public TensorExpression<Derived> {
    public:
        using NotIncludedInScalarArithmetic = void;

        template<typename... I>
        constexpr auto operator()(I&&... indices) const { 
            return eval(Derived::ShapeType::index(std::forward<I>(indices)...));
        }
    };

    template<typename D>
    class TensorWritableExpression : public TensorExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Derived = D;

        template<typename T>
        constexpr Derived& operator=(const TensorWritableExpression<T>& expr) {
            const auto& type = expr.derived();
            std::copy(type.data(), type.data() + derived().size(), derived().data());
            return derived();
        }

        template<typename Der>
        constexpr Derived& operator=(const TensorArithmeticExpression<Der>& expr) {
            for(size_t i = 0; i < derived().size(); ++i) {
                derived().data()[i] = expr.eval(i);
            }
            return derived();
        }

        // Non const access to derived
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }
        static constexpr size_t size() { return Derived::size(); }

        constexpr const auto& eval(size_t i) const { return derived().data()[i]; }
        constexpr const auto& eval() const { return derived(); }

        template<typename... I>
        constexpr auto operator()(I&&... indices) const { 
            return eval(D::ShapeType::index(std::forward<I>(indices)...));
        }

        template<typename... I>
        constexpr auto& operator()(I&&... indices) { 
            return eval(D::ShapeType::index(std::forward<I>(indices)...));
        }
    };

    template<typename D>
    class TensorStorageExpression : public TensorWritableExpression<D> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Derived = D;

        constexpr TensorStorageExpression() = default;
        using TensorWritableExpression<D>::operator=;

        template<typename T>
        constexpr TensorStorageExpression(const TensorWritableExpression<T>& e) {
            const auto& expr = e.derived();
            auto& derived = this->derived();
            std::copy(expr.data(), expr.data() + derived.size(), derived.data());
        }

        template<typename Der>
        constexpr TensorStorageExpression(const TensorArithmeticExpression<Der>& expr) {
            for(size_t i = 0; i < this->derived().size(); ++i) {
                this->derived().data()[i] = expr.eval(i);
            }
        }

        template<typename Ptr>
        explicit constexpr TensorStorageExpression(const Ptr* ptr, size_t size) {
            auto& derived = this->derived();
            std::copy(ptr, ptr + size, derived.data());
        }

        template<typename T>
        constexpr TensorStorageExpression(std::initializer_list<T> list) {
            if(list.size() < size()) {
                std::copy(list.begin(), list.end(), this->derived().data());
            }
            else {
                std::copy(list.begin(), list.begin() + size(), this->derived().data());
            }
        }

        template<typename T>
        constexpr TensorStorageExpression& operator=(std::initializer_list<T> list) {
            if(list.size() < size()) {
                std::copy(list.begin(), list.end(), this->derived().data());
            }
            else {
                std::copy(list.begin(), list.begin() + size(), this->derived().data());
            }
            return *this;
        }

        static constexpr size_t size() { return Derived::size(); }
    };

    enum class BinaryOperationType {
        // Arithmetic operations
        Add,
        Subtract,
        Divide,
        Multiply,
        Modulo,

        // Bitwise operations
        BitwiseAnd,
        BitwiseOr,
        BitwiseXor,
        LeftShift,
        RightShift,

        // Logical operations
        And,
        Or,
        Equal,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual
    };

    template<BinaryOperationType opType, typename L, typename R>
    struct ApplyBinaryOp {
        auto operator()(const L&, const R&) { return L(); }
    };

    // Arithmetic operations
    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Add, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs + rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Subtract, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs - rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Divide, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs / rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Multiply, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs * rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Modulo, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs % rhs; }
    };

    // Bitwise operations
    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::BitwiseAnd, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs & rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::BitwiseOr, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs | rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::BitwiseXor, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs ^ rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::LeftShift, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs << rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::RightShift, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs >> rhs; }
    };

    // Logical operations
    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::And, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs && rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Or, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs || rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Equal, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs == rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::NotEqual, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs != rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Less, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs < rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::LessOrEqual, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs <= rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::Greater, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs > rhs; }
    };

    template<typename L, typename R>
    struct ApplyBinaryOp<BinaryOperationType::GreaterOrEqual, L, R> {
        auto operator()(const L& lhs, const R& rhs) { return lhs >= rhs; }
    };

    template<BinaryOperationType operation, typename L, typename R>
    struct GetValueTypeFromBinaryOperationType {
        using Type = decltype(
            ApplyBinaryOp<operation, typename L::ValueType, typename R::ValueType>{}(
                std::declval<typename L::ValueType>(),
                std::declval<typename R::ValueType>()
            )
        );
    };

    enum class UnaryOperationType {
        // Arithmetic operations
        Negate,

        // Bitwise operations
        BitwiseNegate,

        // Logical operations
        Not,
    };

    template<UnaryOperationType opType, typename T>
    struct ApplyUnaryOp {
        auto operator()(const T&) { return T(); }
    };

    // Arithmetic operations
    template<typename T>
    struct ApplyUnaryOp<UnaryOperationType::Negate, T> {
        auto operator()(const T& val) { return -val; }
    };

    // Bitwise operations
    template<typename T>
    struct ApplyUnaryOp<UnaryOperationType::BitwiseNegate, T> {
        auto operator()(const T& val) { return ~val; }
    };

    // Logical operations
    template<typename T>
    struct ApplyUnaryOp<UnaryOperationType::Not, T> {
        auto operator()(const T& val) { return !val; }
    };

    template<UnaryOperationType operation, typename T>
    struct GetValueTypeFromUnaryOperationType {
        using Type = decltype(ApplyUnaryOp<operation, typename T::ValueType>{}(std::declval<typename T::ValueType>()));
    };

    template<typename ValueType, typename Shape>
    struct FromShape {};

    template<BinaryOperationType operation, typename Shape1, typename Shape2>
    struct BinaryShapeResult {
        static_assert(Shape1::dims() == Shape2::dims(), "Binary operation operands must have the same dimensions");
        using ValueType = GetValueTypeFromBinaryOperationType<operation, Shape1, Shape2>::Type;
        using Tensor = typename FromShape<ValueType, Shape1>::Tensor;
    };

    template<UnaryOperationType operation, typename Shape>
    struct UnaryShapeResult {
        using ValueType = GetValueTypeFromUnaryOperationType<operation, Shape>::Type;
        using Tensor = typename FromShape<ValueType, Shape>::Tensor;
    };

    template<typename T, typename Shape>
    class ScalarExpression : public TensorArithmeticExpression<ScalarExpression<T, Shape>> {
    private:
        T m_value;
    public:
        using NotIncludedInScalarArithmetic = void;
        using ValueType = T;
        using ShapeType = Shape;

        operator T() const { return m_value; }

        static constexpr size_t size() { return ShapeType::size(); }
        constexpr ScalarExpression(T value) : m_value(value) {}
        constexpr auto eval(size_t) const { return m_value; }
        constexpr auto eval() const { return m_value; }
    };

    template<typename L, typename R, BinaryOperationType operation>
    class TensorBinaryExpression : public TensorArithmeticExpression<TensorBinaryExpression<L, R, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ValueType = typename GetValueTypeFromBinaryOperationType<operation, L, R>::Type;
        using TensorType = typename BinaryShapeResult<operation, typename L::ShapeType, typename R::ShapeType>::Tensor;
        using ShapeType = typename TensorType::ShapeType;

    private:
        const L& m_lhs;
        const R& m_rhs;

    public:
        constexpr explicit TensorBinaryExpression(const L& lhs, const R& rhs)
            : m_lhs(lhs), m_rhs(rhs) {}

        static constexpr size_t size() {
            return L::size();
        }

        constexpr ValueType eval(size_t i) const {
            return ApplyBinaryOp<operation, typename L::ValueType, typename R::ValueType>{}(m_lhs.eval(i), m_rhs.eval(i));
        }

        constexpr auto eval() const {
            TensorType result;
            for(size_t i = 0; i < result.size(); ++i)
                result.data()[i] = eval(i);
            return result;
        }
    };

    template<typename T, UnaryOperationType operation>
    class TensorUnaryExpression : public TensorArithmeticExpression<TensorUnaryExpression<T, operation>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ValueType = typename GetValueTypeFromUnaryOperationType<operation, T>::Type;
        using TensorType = typename UnaryShapeResult<operation, typename T::ShapeType>::Tensor;
        using ShapeType = typename TensorType::ShapeType;

    private:
        const T& m_expr;

    public:
        constexpr explicit TensorUnaryExpression(const T& expr) : m_expr(expr){}

        static constexpr size_t size() {
            return T::size();
        }

        constexpr ValueType eval(size_t i) const {
            return ApplyUnaryOp<operation, typename T::ValueType>{}(m_expr.eval(i));
        }

        constexpr auto eval() const {
            TensorType result;
            for(size_t i = 0; i < result.size(); ++i)
                result.data()[i] = eval(i);
            return result;
        }
    };

    enum class ComplexBinaryOperationType {
        Cross
    };

    //L and R are tensors
    template<ComplexBinaryOperationType opType, typename L, typename R>
    struct ApplyComplexBinaryOp {
        auto operator()(size_t, const L&, const R&) { return L(); }
        auto operator()(const L&, const R&) { return L(); }
    };

    template<ComplexBinaryOperationType operation, typename Shape, typename ValueType>
    struct GetTensorForComplexExpression;

    // Complex binary operations
    template<typename L, typename R>
    struct ApplyComplexBinaryOp<ComplexBinaryOperationType::Cross, L, R> {
        static constexpr std::array<std::pair<size_t, size_t>, 3> s_indices = {
            std::make_pair<size_t, size_t>(2, 3),
            std::make_pair<size_t, size_t>(3, 1),
            std::make_pair<size_t, size_t>(1, 2),
        };
        static_assert(L::ShapeType::dims() == R::ShapeType::dims() && L::size() == 3 && L::ShapeType::dimCount() == 1);

        using MultResultType = GetValueTypeFromBinaryOperationType<BinaryOperationType::Multiply, L, R>::Type;
        using SubtractResultType = decltype(
            ApplyBinaryOp<BinaryOperationType::Subtract, MultResultType, MultResultType>{}(
                std::declval<MultResultType>(),
                std::declval<MultResultType>()
            )
        );
        using ValueType = SubtractResultType;
        using TensorType = GetTensorForComplexExpression<ComplexBinaryOperationType::Cross, typename L::ShapeType, ValueType>::Tensor;

        auto operator()(size_t i, const L& lhs, const R& rhs) { 
            return lhs.eval(s_indices[i].first) * rhs.eval(s_indices[i].second) - 
                lhs.eval(s_indices[i].second) * rhs.eval(s_indices[i].first);
        }
        auto operator()(const L& lhs, const R& rhs) { 
            TensorType result;
            result[0] = lhs.eval(2) * rhs.eval(3) - lhs.eval(3) * rhs.eval(2);
            result[1] = lhs.eval(3) * rhs.eval(1) - lhs.eval(1) * rhs.eval(3);
            result[2] = lhs.eval(1) * rhs.eval(2) - lhs.eval(2) * rhs.eval(1);
            return result;
        }
    };

    template<ComplexBinaryOperationType operation, typename L, typename R>
    class TensorComplexExpression : public TensorArithmeticExpression<TensorComplexExpression<operation, L, R>> {
    public:
        static_assert(L::ShapeType::dims() == R::ShapeType::dims());

        using NotIncludedInScalarArithmetic = void;
        using OperationApplier = ApplyComplexBinaryOp<operation, L, R>;
        using ValueType = typename OperationApplier::ValueType;
        using TensorType = typename OperationApplier::TensorType;
        using ShapeType = typename TensorType::ShapeType;

    private:
        const L& m_lhs;
        const R& m_rhs;

    public:
        constexpr explicit TensorComplexExpression(const L& lhs, const R& rhs)
            : m_lhs(lhs), m_rhs(rhs) {}

        static constexpr size_t size() { return L::size(); }

        constexpr auto eval(size_t i) const { return OperationApplier{}(i, m_lhs, m_rhs); }
        constexpr auto eval() const { return OperationApplier{}(m_lhs, m_rhs); }
    };

    template<typename Derived, typename T, size_t... dims>
    class TensorBase;

    template<typename T, size_t... dims>
    class TensorView : public TensorBase<TensorView<T, dims...>, T, dims...>, public TensorWritableExpression<TensorView<T, dims...>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Base = TensorBase<TensorView<T, dims...>, T, dims...>;
        using BaseExpr = TensorWritableExpression<TensorView<T, dims...>>;
        using ValueType = typename Base::ValueType;
        using ShapeType = typename Base::ShapeType;

    private:
        T* m_data;

    public:
        template<typename U>
        constexpr explicit TensorView(U* ptr) requires std::same_as<std::remove_cv_t<T>, U> : m_data(ptr) {}

        template<typename U>
        constexpr TensorView(TensorView<U, dims...> view) requires std::same_as<std::remove_cv_t<T>, U> : m_data(view.data()) {}

        using BaseExpr::operator=;
        using BaseExpr::BaseExpr;
        using Base::size;
        using Base::operator[];

        constexpr TensorView<T, dims...> view() { return TensorView<T, dims...>(m_data); }
        constexpr TensorView<const T, dims...> view() const { return TensorView<const T, dims...>(m_data); }
        
        constexpr const T* data() const { return m_data; }
        constexpr T* data() { return m_data; }
    };

    template<typename T, size_t... dims>
    class Tensor : public TensorBase<Tensor<T, dims...>, T, dims...>, public TensorStorageExpression<Tensor<T, dims...>> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using Base = TensorBase<Tensor<T, dims...>, T, dims...>;
        using BaseExpr = TensorStorageExpression<Tensor<T, dims...>>;
        using ValueType = typename Base::ValueType;
        using ShapeType = typename Base::ShapeType;

    private:
        std::array<T, Base::size()> m_storage{};

    public:
        constexpr Tensor() = default;

        using BaseExpr::operator=;
        using BaseExpr::BaseExpr;
        using Base::size;
        using Base::operator[];

        constexpr Tensor(TensorView<T, dims...> view) : BaseExpr(view.data(), view.size()) {};
        constexpr Tensor& operator=(TensorView<T, dims...> view) { 
            BaseExpr::operator=(view);
            return *this; 
        };

        constexpr operator TensorView<T, dims...>() { return view(); }
        constexpr operator TensorView<const T, dims...>() const { return view(); }

        constexpr TensorView<T, dims...> view() { return TensorView<T, dims...>(m_storage.data()); }
        constexpr TensorView<const T, dims...> view() const { return TensorView<const T, dims...>(m_storage.data()); }

        constexpr const T* data() const { return m_storage.data(); }
        constexpr T* data() { return m_storage.data(); }
    };

    template<typename Derived, typename T, size_t N>
    class TensorBase<Derived, T, N> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ValueType = T;
        using ShapeType = Shape<T, N>;

        static constexpr size_t size() { return ShapeType::size(); }
        static constexpr size_t stride() { return ShapeType::stride(); }
        static constexpr const auto& dims() { return ShapeType::dims(); }
        static constexpr size_t dimCount() { return ShapeType::dimCount(); }

        static constexpr size_t outerSize() { return N; }

        constexpr T& operator[](size_t i) { return this->derived().data()[i]; }
        constexpr const T& operator[](size_t i) const { return this->derived().data()[i]; }

    protected:
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }
    };

    template<typename Derived, typename T, size_t N, size_t... rest>
    class TensorBase<Derived, T, N, rest...> {
    public:
        using NotIncludedInScalarArithmetic = void;
        using ValueType = T;
        using ShapeType = Shape<T, N, rest...>;

    public:
        static constexpr size_t size() { return ShapeType::size(); }
        static constexpr size_t stride() { return ShapeType::stride(); }
        static constexpr const auto& dims() { return ShapeType::dims(); }
        static constexpr size_t dimCount() { return ShapeType::dimCount(); }

        static constexpr size_t outerSize() { return N; }

        constexpr auto operator[](size_t i) { return TensorView<T, rest...>(derived().data() + i * ShapeType::stride()); }
        constexpr auto operator[](size_t i) const { return TensorView<const T, rest...>(derived().data() + i * ShapeType::stride()); }

    protected:
        constexpr Derived& derived() { return static_cast<Derived&>(*this); }
        constexpr const Derived& derived() const { return static_cast<const Derived&>(*this); }
    };

    template<typename ValueType, typename T, size_t... dims>
    struct FromShape<ValueType, Shape<T, dims...>> {
        using Tensor = Tensor<ValueType, dims...>;
    };

    template<typename Shape, typename ValueType>
    struct GetTensorForComplexExpression<ComplexBinaryOperationType::Cross, Shape, ValueType> {
        using Tensor = Tensor<ValueType, 3>;
    };
}
