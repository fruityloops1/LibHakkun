#pragma once

#include "hk/prim/traits/Integer.h"

#if __cplusplus >= 202400 and not __clang__
#define HK_REFLECTION 1
#else
#define HK_REFLECTION 0
#endif

#if HK_REFLECTION

#define reflexpr(...) ^^__VA_ARGS__

namespace hk {

    using ReflectInfo = decltype(reflexpr(int));

} // namespace hk

#include <inplace_vector>
#include <optional>
#include <string>
#include <vector>

namespace std::meta {

    using info = hk::ReflectInfo;

    consteval ::size size_of(info);
    consteval ::size bit_size_of(info);
    consteval ::size alignment_of(info);

    consteval bool is_public(info);
    consteval bool is_protected(info);
    consteval bool is_private(info);

    consteval bool is_virtual(info);
    consteval bool is_pure_virtual(info);
    consteval bool is_override(info);
    consteval bool is_final(info);

    consteval bool is_deleted(info);
    consteval bool is_defaulted(info);
    consteval bool is_user_provided(info);
    consteval bool is_user_declared(info);
    consteval bool is_explicit(info);
    consteval bool is_noexcept(info);

    consteval bool is_bit_field(info);
    consteval bool is_enumerator(info);
    consteval bool is_annotation(info);

    consteval bool is_const(info);
    consteval bool is_volatile(info);
    consteval bool is_mutable_member(info);
    consteval bool is_lvalue_reference_qualified(info);
    consteval bool is_rvalue_reference_qualified(info);

    consteval bool has_static_storage_duration(info);
    consteval bool has_thread_storage_duration(info);
    consteval bool has_automatic_storage_duration(info);

    consteval bool has_internal_linkage(info);
    consteval bool has_module_linkage(info);
    consteval bool has_external_linkage(info);
    consteval bool has_c_language_linkage(info);
    consteval bool has_linkage(info);

    consteval bool is_complete_type(info);
    consteval bool is_enumerable_type(info);

    consteval bool is_variable(info);
    consteval bool is_type(info);
    consteval bool is_namespace(info);
    consteval bool is_type_alias(info);
    consteval bool is_namespace_alias(info);

    consteval bool is_function(info);
    consteval bool is_conversion_function(info);
    consteval bool is_operator_function(info);
    consteval bool is_literal_operator(info);
    consteval bool is_special_member_function(info);
    consteval bool is_constructor(info);
    consteval bool is_default_constructor(info);
    consteval bool is_copy_constructor(info);
    consteval bool is_move_constructor(info);
    consteval bool is_assignment(info);
    consteval bool is_copy_assignment(info);
    consteval bool is_move_assignment(info);
    consteval bool is_destructor(info);

    consteval bool is_function_parameter(info);
    consteval bool is_explicit_object_parameter(info);
    consteval bool has_default_argument(info);
    consteval bool is_vararg_function(info);

    consteval bool is_template(info);
    consteval bool is_function_template(info);
    consteval bool is_variable_template(info);
    consteval bool is_class_template(info);
    consteval bool is_alias_template(info);
    consteval bool is_conversion_function_template(info);
    consteval bool is_operator_function_template(info);
    consteval bool is_literal_operator_template(info);
    consteval bool is_constructor_template(info);
    consteval bool is_concept(info);

    consteval bool is_value(info);
    consteval bool is_object(info);

    consteval bool is_structured_binding(info);

    consteval bool is_class_member(info);
    consteval bool is_namespace_member(info);
    consteval bool is_nonstatic_data_member(info);
    consteval bool is_static_member(info);
    consteval bool is_base(info);

    struct data_member_options {
        struct _Name {
            template <class _Tp>
                requires constructible_from<u8string, _Tp>
            consteval _Name(_Tp&& __n)
                : _M_is_u8(true)
                , _M_u8s((_Tp&&)__n) { }

            template <class _Tp>
                requires constructible_from<string, _Tp>
            consteval _Name(_Tp&& __n)
                : _M_is_u8(false)
                , _M_s((_Tp&&)__n) { }

        private:
            bool _M_is_u8;
            u8string _M_u8s;
            string _M_s;
            info _M_unused = { };
        };

        optional<_Name> name;
        optional<int> alignment = { };
        optional<int> bit_width = { };
        bool no_unique_address = false;
        vector<info> annotations = { };
    };
    consteval info data_member_spec(info, data_member_options);
    consteval bool is_data_member_spec(info);
    // template<reflection_range _Rg = initializer_list<info>>
    template <typename Range>
    consteval info define_aggregate(info, Range&&);

    struct access_context {
        consteval access_context(info __scope, info __designating_class) noexcept
            : _M_scope { __scope }
            , _M_designating_class { __designating_class } { }

        consteval info scope() const { return _M_scope; }
        consteval info designating_class() const { return _M_designating_class; }

        static consteval access_context current() noexcept;
        static consteval access_context unprivileged() noexcept { return access_context { ^^::, info { } }; }
        static consteval access_context unchecked() noexcept { return access_context { info { }, info { } }; }
        consteval access_context via(info) const;

        info _M_scope;
        info _M_designating_class;
    };

    consteval bool is_accessible(info, access_context);
    consteval bool has_inaccessible_nonstatic_data_members(info,
        access_context);
    consteval bool has_inaccessible_bases(info, access_context);
    consteval bool has_inaccessible_subobjects(info, access_context);

    consteval info current_function();
    consteval info current_class();
    consteval info current_namespace();

    consteval vector<info> members_of(info, access_context);
    consteval vector<info> bases_of(info, access_context);
    consteval vector<info> static_data_members_of(info, access_context);
    consteval vector<info> nonstatic_data_members_of(info, access_context);
    consteval vector<info> subobjects_of(info, access_context);
    consteval vector<info> enumerators_of(info);

    consteval string_view identifier_of(info);
    consteval u8string_view u8identifier_of(info);

    consteval string_view display_string_of(info);
    consteval u8string_view u8display_string_of(info);

} // namespace std::meta

namespace hk {

    class Reflect;

    class ReflectAccessContext {
        ReflectInfo mScope;
        ReflectInfo mDesignatingClass;

        consteval ReflectAccessContext(std::meta::access_context context)
            : mScope(context.scope())
            , mDesignatingClass(context.designating_class()) { }

    public:
        consteval ReflectAccessContext() = delete;
        consteval ReflectAccessContext(const ReflectAccessContext&) = default;
        consteval ReflectAccessContext(ReflectAccessContext&&) = default;

        consteval operator std::meta::access_context() const { return { mScope, mDesignatingClass }; }

        consteval static ReflectAccessContext getCurrent() { return std::meta::access_context::current(); }
        consteval static ReflectAccessContext getUnprivileged() { return std::meta::access_context::unprivileged(); }
        consteval static ReflectAccessContext getUnchecked() { return std::meta::access_context::unchecked(); }
        consteval ReflectAccessContext via(Reflect info);
    };

    struct Reflect {
        ReflectInfo mValue;

        consteval Reflect() = default;
        consteval Reflect(const ReflectInfo& value)
            : mValue(value) { }

        consteval operator ReflectInfo() const { return mValue; }

        consteval size getSize() const { return std::meta::size_of(mValue); }
        consteval size getSizeBits() const { return std::meta::bit_size_of(mValue); }
        consteval size getAlignment() const { return std::meta::alignment_of(mValue); }

        consteval std::string_view getIdentifier() { return std::meta::identifier_of(mValue); }
        consteval std::u8string_view getU8Identifier() { return std::meta::u8identifier_of(mValue); }

        consteval std::string_view getDisplayString() { return std::meta::display_string_of(mValue); }
        consteval std::u8string_view getU8DisplayString() { return std::meta::u8display_string_of(mValue); }

        consteval bool isPublic() const { return std::meta::is_public(mValue); }
        consteval bool isProtected() const { return std::meta::is_protected(mValue); }
        consteval bool isPrivate() const { return std::meta::is_private(mValue); }

        consteval bool isVirtual() const { return std::meta::is_virtual(mValue); }
        consteval bool isPureVirtual() const { return std::meta::is_pure_virtual(mValue); }
        consteval bool isOverride() const { return std::meta::is_override(mValue); }
        consteval bool isFinal() const { return std::meta::is_final(mValue); }

        consteval bool isDeleted() const { return std::meta::is_deleted(mValue); }
        consteval bool isDefaulted() const { return std::meta::is_defaulted(mValue); }
        consteval bool isUserProvided() const { return std::meta::is_user_provided(mValue); }
        consteval bool isUserDeclared() const { return std::meta::is_user_declared(mValue); }
        consteval bool isExplicit() const { return std::meta::is_explicit(mValue); }
        consteval bool isNoexcept() const { return std::meta::is_noexcept(mValue); }

        consteval bool isBitField() const { return std::meta::is_bit_field(mValue); }
        consteval bool isEnumerator() const { return std::meta::is_enumerator(mValue); }
        consteval bool isAnnotation() const { return std::meta::is_annotation(mValue); }

        consteval bool isConst() const { return std::meta::is_const(mValue); }
        consteval bool isVolatile() const { return std::meta::is_volatile(mValue); }
        consteval bool isMutableMember() const { return std::meta::is_mutable_member(mValue); }
        consteval bool isLvalueReferenceQualified() const { return std::meta::is_lvalue_reference_qualified(mValue); }
        consteval bool isRvalueReferenceQualified() const { return std::meta::is_rvalue_reference_qualified(mValue); }

        consteval bool hasStaticStorageDuration() const { return std::meta::has_static_storage_duration(mValue); }
        consteval bool hasThreadStorageDuration() const { return std::meta::has_thread_storage_duration(mValue); }
        consteval bool hasAutomaticStorageDuration() const { return std::meta::has_automatic_storage_duration(mValue); }

        consteval bool hasInternalLinkage() const { return std::meta::has_internal_linkage(mValue); }
        consteval bool hasModuleLinkage() const { return std::meta::has_module_linkage(mValue); }
        consteval bool hasExternalLinkage() const { return std::meta::has_external_linkage(mValue); }
        consteval bool hasCLanguageLinkage() const { return std::meta::has_c_language_linkage(mValue); }
        consteval bool hasLinkage() const { return std::meta::has_linkage(mValue); }

        consteval bool isCompleteType() const { return std::meta::is_complete_type(mValue); }
        consteval bool isEnumerableType() const { return std::meta::is_enumerable_type(mValue); }

        consteval bool isVariable() const { return std::meta::is_variable(mValue); }
        consteval bool isType() const { return std::meta::is_type(mValue); }
        consteval bool isNamespace() const { return std::meta::is_namespace(mValue); }
        consteval bool isTypeAlias() const { return std::meta::is_type_alias(mValue); }
        consteval bool isNamespaceAlias() const { return std::meta::is_namespace_alias(mValue); }

        consteval bool isFunction() const { return std::meta::is_function(mValue); }
        consteval bool isConversionFunction() const { return std::meta::is_conversion_function(mValue); }
        consteval bool isOperatorFunction() const { return std::meta::is_operator_function(mValue); }
        consteval bool isLiteralOperator() const { return std::meta::is_literal_operator(mValue); }
        consteval bool isSpecialMemberFunction() const { return std::meta::is_special_member_function(mValue); }
        consteval bool isConstructor() const { return std::meta::is_constructor(mValue); }
        consteval bool isDefaultConstructor() const { return std::meta::is_default_constructor(mValue); }
        consteval bool isCopyConstructor() const { return std::meta::is_copy_constructor(mValue); }
        consteval bool isMoveConstructor() const { return std::meta::is_move_constructor(mValue); }
        consteval bool isAssignment() const { return std::meta::is_assignment(mValue); }
        consteval bool isCopyAssignment() const { return std::meta::is_copy_assignment(mValue); }
        consteval bool isMoveAssignment() const { return std::meta::is_move_assignment(mValue); }
        consteval bool isDestructor() const { return std::meta::is_destructor(mValue); }

        consteval bool isFunctionParameter() const { return std::meta::is_function_parameter(mValue); }
        consteval bool isExplicitObjectParameter() const { return std::meta::is_explicit_object_parameter(mValue); }
        consteval bool hasDefaultArgument() const { return std::meta::has_default_argument(mValue); }
        consteval bool isVarargFunction() const { return std::meta::is_vararg_function(mValue); }

        consteval bool isTemplate() const { return std::meta::is_template(mValue); }
        consteval bool isFunctionTemplate() const { return std::meta::is_function_template(mValue); }
        consteval bool isVariableTemplate() const { return std::meta::is_variable_template(mValue); }
        consteval bool isClassTemplate() const { return std::meta::is_class_template(mValue); }
        consteval bool isAliasTemplate() const { return std::meta::is_alias_template(mValue); }
        consteval bool isConversionFunctionTemplate() const { return std::meta::is_conversion_function_template(mValue); }
        consteval bool isOperatorFunctionTemplate() const { return std::meta::is_operator_function_template(mValue); }
        consteval bool isLiteralOperatorTemplate() const { return std::meta::is_literal_operator_template(mValue); }
        consteval bool isConstructorTemplate() const { return std::meta::is_constructor_template(mValue); }
        consteval bool isConcept() const { return std::meta::is_concept(mValue); }

        consteval bool isValue() const { return std::meta::is_value(mValue); }
        consteval bool isObject() const { return std::meta::is_object(mValue); }

        consteval bool isStructuredBinding() const { return std::meta::is_structured_binding(mValue); }

        consteval bool isClassMember() const { return std::meta::is_class_member(mValue); }
        consteval bool isNamespaceMember() const { return std::meta::is_namespace_member(mValue); }
        consteval bool isNonstaticDataMember() const { return std::meta::is_nonstatic_data_member(mValue); }
        consteval bool isStaticMember() const { return std::meta::is_static_member(mValue); }
        consteval bool isBase() const { return std::meta::is_base(mValue); }

        template <typename C>
        consteval void defineAggregate(C&& dataMembers) {
            std::vector<ReflectInfo> members;
            for (ReflectInfo member : dataMembers)
                members.push_back(member);

            std::meta::define_aggregate(mValue, members);
        }

        consteval bool isDataMemberDescription() const { return std::meta::is_data_member_spec(mValue); }

        template <typename C = int>
        consteval Reflect makeDataMemberDescription(const char* name, int alignment = -1, int bitWidth = -1, bool noUniqueAddress = false, C&& annotations = 0) const {
            std::meta::data_member_options options;
            options.name = std::meta::data_member_options::_Name(name);
            options.alignment = int(alignment == -1 ? int(getAlignment()) : alignment);
            if (bitWidth != -1)
                options.bit_width = bitWidth;
            options.no_unique_address = noUniqueAddress;

            std::vector<ReflectInfo> annotationsVec;

            if constexpr (!util::ctIsSame<C, int>) {
                for (ReflectInfo attr : annotations)
                    annotationsVec.push_back(attr);
            }

            options.annotations = annotationsVec;

            return { std::meta::data_member_spec(mValue, options) };
        }

        consteval bool isAccessible(ReflectAccessContext context) const { return std::meta::is_accessible(mValue, context); }
        consteval bool hasInaccessibleNonstaticDataMembers(ReflectAccessContext context) const { return std::meta::has_inaccessible_nonstatic_data_members(mValue, context); }
        consteval bool hasInaccessibleBases(ReflectAccessContext context) const { return std::meta::has_inaccessible_bases(mValue, context); }
        consteval bool hasInaccessibleSubobjects(ReflectAccessContext context) const { return std::meta::has_inaccessible_subobjects(mValue, context); }

        struct ReflectVec;

        consteval ReflectVec getMembers(ReflectAccessContext context = hk::ReflectAccessContext::getCurrent()) const;
        consteval ReflectVec getBases(ReflectAccessContext context) const;
        consteval ReflectVec getStaticDataMembers(ReflectAccessContext context = hk::ReflectAccessContext::getCurrent()) const;
        consteval ReflectVec getNonStaticDataMembers(ReflectAccessContext context = hk::ReflectAccessContext::getCurrent()) const;
        consteval ReflectVec getSubobjects(ReflectAccessContext context) const;
        consteval ReflectVec getEnumerators() const;
    };

    struct Reflect::ReflectVec {
        constexpr static size cCapacity = 0x10000;
        Reflect data[cCapacity];
        ::size size = 0;

        consteval ReflectVec() = default;
        consteval Reflect operator[](::size idx) const {
            if (idx >= this->size)
                throw std::runtime_error("");
            return this->data[idx];
        }

        consteval void add(Reflect elem) {
            if (this->size >= cCapacity)
                throw std::runtime_error("");
            this->data[this->size++] = elem;
        }

        consteval const Reflect* begin() const { return this->data; }
        consteval const Reflect* end() const { return this->data + this->size; }
    };

    consteval Reflect::ReflectVec Reflect::getMembers(ReflectAccessContext context) const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::members_of(mValue, context))
            out.add(info);
        return out;
    }

    consteval Reflect::ReflectVec Reflect::getBases(ReflectAccessContext context) const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::bases_of(mValue, context))
            out.add(info);
        return out;
    }

    consteval Reflect::ReflectVec Reflect::getStaticDataMembers(ReflectAccessContext context) const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::static_data_members_of(mValue, context))
            out.add(info);
        return out;
    }

    consteval Reflect::ReflectVec Reflect::getNonStaticDataMembers(ReflectAccessContext context) const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::nonstatic_data_members_of(mValue, context))
            out.add(info);
        return out;
    }

    consteval Reflect::ReflectVec Reflect::getSubobjects(ReflectAccessContext context) const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::subobjects_of(mValue, context))
            out.add(info);
        return out;
    }

    consteval Reflect::ReflectVec Reflect::getEnumerators() const {
        ReflectVec out;
        for (ReflectInfo info : std::meta::enumerators_of(mValue))
            out.add(info);
        return out;
    }

    consteval ReflectAccessContext ReflectAccessContext::via(Reflect info) {
        return std::meta::access_context(*this).via(info);
    }

} // namespace hk

#endif