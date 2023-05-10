// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: size.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_size_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_size_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021009 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_size_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_size_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_size_2eproto;
namespace frame {
namespace proto {
class Size;
struct SizeDefaultTypeInternal;
extern SizeDefaultTypeInternal _Size_default_instance_;
}  // namespace proto
}  // namespace frame
PROTOBUF_NAMESPACE_OPEN
template<> ::frame::proto::Size* Arena::CreateMaybeMessage<::frame::proto::Size>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace frame {
namespace proto {

// ===================================================================

class Size final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:frame.proto.Size) */ {
 public:
  inline Size() : Size(nullptr) {}
  ~Size() override;
  explicit PROTOBUF_CONSTEXPR Size(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Size(const Size& from);
  Size(Size&& from) noexcept
    : Size() {
    *this = ::std::move(from);
  }

  inline Size& operator=(const Size& from) {
    CopyFrom(from);
    return *this;
  }
  inline Size& operator=(Size&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Size& default_instance() {
    return *internal_default_instance();
  }
  static inline const Size* internal_default_instance() {
    return reinterpret_cast<const Size*>(
               &_Size_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(Size& a, Size& b) {
    a.Swap(&b);
  }
  inline void Swap(Size* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Size* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Size* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Size>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Size& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Size& from) {
    Size::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Size* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "frame.proto.Size";
  }
  protected:
  explicit Size(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kXFieldNumber = 1,
    kYFieldNumber = 2,
  };
  // int32 x = 1;
  void clear_x();
  int32_t x() const;
  void set_x(int32_t value);
  private:
  int32_t _internal_x() const;
  void _internal_set_x(int32_t value);
  public:

  // int32 y = 2;
  void clear_y();
  int32_t y() const;
  void set_y(int32_t value);
  private:
  int32_t _internal_y() const;
  void _internal_set_y(int32_t value);
  public:

  // @@protoc_insertion_point(class_scope:frame.proto.Size)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    int32_t x_;
    int32_t y_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_size_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Size

// int32 x = 1;
inline void Size::clear_x() {
  _impl_.x_ = 0;
}
inline int32_t Size::_internal_x() const {
  return _impl_.x_;
}
inline int32_t Size::x() const {
  // @@protoc_insertion_point(field_get:frame.proto.Size.x)
  return _internal_x();
}
inline void Size::_internal_set_x(int32_t value) {
  
  _impl_.x_ = value;
}
inline void Size::set_x(int32_t value) {
  _internal_set_x(value);
  // @@protoc_insertion_point(field_set:frame.proto.Size.x)
}

// int32 y = 2;
inline void Size::clear_y() {
  _impl_.y_ = 0;
}
inline int32_t Size::_internal_y() const {
  return _impl_.y_;
}
inline int32_t Size::y() const {
  // @@protoc_insertion_point(field_get:frame.proto.Size.y)
  return _internal_y();
}
inline void Size::_internal_set_y(int32_t value) {
  
  _impl_.y_ = value;
}
inline void Size::set_y(int32_t value) {
  _internal_set_y(value);
  // @@protoc_insertion_point(field_set:frame.proto.Size.y)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace proto
}  // namespace frame

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_size_2eproto
