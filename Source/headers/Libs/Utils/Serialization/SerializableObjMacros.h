#ifndef SERIALIZABLEOBJMACROS_H
#define SERIALIZABLEOBJMACROS_H


#define SERIALIZABLE_OBJECT_PRV_(ClassName) \
    struct ClassName { \
        friend struct thaf::srz::Serialization<ClassName>; \
        DEFAULT_CONSTRUCTIONS_(ClassName)\
        MAKE_SERIALIZABLE_(ClassName)

#define SERIALIZABLE_OBJECT_END_PRV_(ClassName) };

#define HAS_PROPERTIES_PRV_(MemberNameList, ...) \
        private: \
        enum { \
            MemberNameList, __VA_ARGS__ \
        };

#define WITH_RESPECTIVE_TYPES_PRV_(...) \
        public: \
        using value_type = std::tuple<__VA_ARGS__>; \
        private: \
        std::tuple<__VA_ARGS__> _data; \
        public: \
        std::tuple<__VA_ARGS__>& data() { return _data; } \
        const std::tuple<__VA_ARGS__>& data() const { return _data; }

#define DEFINE_GET_SET_METHODS_PRV_(Type, Name) \
        public: \
        const Type& get##Name() const { \
            return std::get<Name>(this->_data); \
        } \
        void set##Name(const Type& value) \
        { \
            std::get<Name>(this->_data) = value; \
        } \
        void set##Name(Type&& value) \
        { \
            std::get<Name>(this->_data) = std::move(value); \
        }

#define DEFAULT_CONSTRUCTIONS_(ClassName) \
        ClassName() = default; \
        ClassName(const ClassName& rhs) = default; \
        ClassName(ClassName&& rhs) = default; \
        ClassName& operator=(const ClassName& rhs) = default; \
        ClassName& operator=(ClassName&& rhs) = default; \
        template <typename... T> \
        ClassName(T&&... args) : _data(std::forward<T>(args)...) {}

#define MAKE_SERIALIZABLE_(ClassName)/*\
    template<class OStream> \
    friend inline thaf::srz::StreamSerializer<OStream>& operator<<(thaf::srz::StreamSerializer<OStream>& srzr, const ClassName& thisObject) { \
        thaf::stl::tuple_for_each(thisObject._data, [&srzr](const auto& e) { srzr << e; }); \
        return srzr; \
    }\
        friend inline thaf::srz::Deserializer& operator>>(thaf::srz::Deserializer& dszr, ClassName& thisObject) \
        { \
            thaf::stl::tuple_for_each(thisObject._data, [&dszr](auto& e) { dszr >> e; }); \
            return dszr; \
        }*/

#endif // SERIALIZABLEOBJMACROS_H
