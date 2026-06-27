#pragma once

#include <array>
#include <cstring>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace bse {

using ObjectId = std::uint32_t;
constexpr ObjectId kInvalidObjectId = std::numeric_limits<ObjectId>::max();

struct Vec2 { float x = 0.0f; float y = 0.0f; };
struct Vec3 { float x = 0.0f; float y = 0.0f; float z = 0.0f; };
struct Vec4 { float x = 0.0f; float y = 0.0f; float z = 0.0f; float w = 1.0f; };

struct Mat4 {
    std::array<float, 16> m{};
    static Mat4 identity();
    static Mat4 translation(const Vec3& value);
    static Mat4 scale(const Vec3& value);
    Mat4 operator*(const Mat4& rhs) const;
};

struct Transform {
    Vec3 translation{0.0f, 0.0f, 0.0f};
    Vec4 rotation{0.0f, 0.0f, 0.0f, 1.0f};
    Vec3 scale{1.0f, 1.0f, 1.0f};
    Mat4 toMatrix() const;
};

inline bool isValidId(ObjectId id) { return id != kInvalidObjectId; }

enum class ErrorCode {
    None,
    IoError,
    TruncatedInput,
    InvalidMagic,
    UnsupportedVersion,
    InvalidSectionTable,
    InvalidOffset,
    InvalidObject,
    MissingReference,
    CompressionError,
    ValidationError,
    InternalError
};

struct Error {
    ErrorCode code = ErrorCode::None;
    std::string message;
    std::string context;
    explicit operator bool() const { return code != ErrorCode::None; }
};

class ErrorList {
public:
    void add(ErrorCode code, std::string message, std::string context = {});
    bool empty() const;
    std::size_t size() const;
    const std::vector<Error>& entries() const;
    std::string format() const;

private:
    std::vector<Error> entries_;
};

template <typename T>
class Result {
public:
    Result(T value) : ok_(true), value_(std::move(value)) {}
    Result(Error error) : ok_(false), error_(std::move(error)) {}
    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }
    T& value() { return value_; }
    const T& value() const { return value_; }
    const Error& error() const { return error_; }

private:
    bool ok_ = false;
    T value_{};
    Error error_{};
};

template <>
class Result<void> {
public:
    Result() : ok_(true) {}
    Result(Error error) : ok_(false), error_(std::move(error)) {}
    bool ok() const { return ok_; }
    explicit operator bool() const { return ok_; }
    const Error& error() const { return error_; }

private:
    bool ok_ = false;
    Error error_{};
};

enum class LogLevel { Verbose, Debug, Info, Warning, Error };

struct LogRecord {
    LogLevel level;
    std::string channel;
    std::string message;
};

class Logger {
public:
    using Sink = std::function<void(const LogRecord&)>;
    void setLevel(LogLevel level);
    void addSink(Sink sink);
    void clearSinks();
    void log(LogLevel level, std::string channel, std::string message);

private:
    std::mutex mutex_;
    LogLevel level_ = LogLevel::Info;
    std::vector<Sink> sinks_;
};

Logger& defaultLogger();

class BinaryReader {
public:
    BinaryReader(const std::uint8_t* data, std::size_t size);
    explicit BinaryReader(const std::vector<std::uint8_t>& data);
    std::size_t position() const;
    std::size_t remaining() const;
    bool seek(std::size_t offset);
    bool canRead(std::size_t count) const;

    template <typename T>
    Result<T> readPod() {
        if (!canRead(sizeof(T))) {
            return Error{ErrorCode::TruncatedInput, "not enough input bytes", "BinaryReader"};
        }
        T value{};
        std::memcpy(&value, data_ + position_, sizeof(T));
        position_ += sizeof(T);
        return value;
    }

    Result<std::vector<std::uint8_t>> readBytes(std::size_t count);
    Result<std::string> readString(std::size_t count);

private:
    const std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t position_ = 0;
};

class BinaryWriter {
public:
    std::size_t position() const;
    const std::vector<std::uint8_t>& bytes() const;
    std::vector<std::uint8_t>& bytes();

    template <typename T>
    void writePod(const T& value) {
        const auto* raw = reinterpret_cast<const std::uint8_t*>(&value);
        data_.insert(data_.end(), raw, raw + sizeof(T));
    }

    void writeBytes(const std::vector<std::uint8_t>& bytes);
    void writeBytes(const std::uint8_t* bytes, std::size_t size);
    void writeString(const std::string& text);
    void align(std::size_t alignment, std::uint8_t pad = 0);
    void patch(std::size_t offset, const void* data, std::size_t size);

private:
    std::vector<std::uint8_t> data_;
};

namespace format {

constexpr std::array<std::uint8_t, 4> kMagic{{'B', 'S', 'E', 'N'}};
constexpr std::uint16_t kMajorVersion = 1;
constexpr std::uint16_t kMinorVersion = 0;
constexpr std::uint32_t kAlignment = 8;

enum class SectionKind : std::uint32_t {
    Strings = 1,
    Objects = 2,
    Meshes = 3,
    Materials = 4,
    Textures = 5,
    Animations = 6,
    Skeletons = 7,
    Nodes = 8,
    Metadata = 9,
    Cameras = 10,
    Lights = 11
};

enum SectionFlags : std::uint32_t {
    SectionFlagNone = 0,
    SectionFlagCompressed = 1 << 0,
    SectionFlagOptional = 1 << 1
};

enum class ObjectKind : std::uint32_t {
    Mesh = 1,
    Material = 2,
    Texture = 3,
    Animation = 4,
    Skeleton = 5,
    Node = 6,
    Camera = 7,
    Light = 8
};

#pragma pack(push, 1)
struct FileHeader {
    std::uint8_t magic[4];
    std::uint16_t majorVersion;
    std::uint16_t minorVersion;
    std::uint32_t headerSize;
    std::uint32_t sectionCount;
    std::uint64_t sectionTableOffset;
    std::uint64_t objectTableOffset;
    std::uint64_t stringTableOffset;
    std::uint64_t fileSize;
    std::uint32_t checksum;
    std::uint32_t flags;
};

struct SectionRecord {
    std::uint32_t kind;
    std::uint32_t flags;
    std::uint64_t offset;
    std::uint64_t size;
    std::uint64_t uncompressedSize;
};

struct ObjectRecord {
    std::uint32_t id;
    std::uint32_t kind;
    std::uint64_t offset;
    std::uint64_t size;
};
#pragma pack(pop)

const char* sectionName(SectionKind kind);
bool isKnownSection(std::uint32_t raw);
std::uint64_t alignTo(std::uint64_t value, std::uint64_t alignment = kAlignment);

} // namespace format

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Vec2 texcoord;
};

class Mesh {
public:
    Mesh() = default;
    Mesh(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    std::vector<Vertex>& vertices();
    const std::vector<Vertex>& vertices() const;
    std::vector<std::uint32_t>& indices();
    const std::vector<std::uint32_t>& indices() const;
    void setMaterialId(ObjectId id);
    ObjectId materialId() const;
    bool isIndexedTriangleMesh() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    std::vector<Vertex> vertices_;
    std::vector<std::uint32_t> indices_;
    ObjectId materialId_ = kInvalidObjectId;
};

enum class TextureFormat : std::uint32_t { Unknown, R8, Rgba8, Bc1, Bc7 };

class TextureMetadata {
public:
    TextureMetadata() = default;
    TextureMetadata(ObjectId id, std::string name, std::string uri);
    ObjectId id() const;
    const std::string& name() const;
    const std::string& uri() const;
    void setDimensions(std::uint32_t width, std::uint32_t height);
    std::uint32_t width() const;
    std::uint32_t height() const;
    void setFormat(TextureFormat format);
    TextureFormat format() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    std::string uri_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    TextureFormat format_ = TextureFormat::Unknown;
};

class Material {
public:
    Material() = default;
    Material(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    void setBaseColor(Vec4 color);
    Vec4 baseColor() const;
    void setMetallic(float value);
    float metallic() const;
    void setRoughness(float value);
    float roughness() const;
    void setTextureSlot(std::string slot, ObjectId textureId);
    ObjectId textureForSlot(const std::string& slot) const;
    const std::unordered_map<std::string, ObjectId>& textureSlots() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    Vec4 baseColor_{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic_ = 0.0f;
    float roughness_ = 0.5f;
    std::unordered_map<std::string, ObjectId> textureSlots_;
};

struct KeyframeVec3 { float time = 0.0f; Vec3 value; };
struct KeyframeVec4 { float time = 0.0f; Vec4 value; };

class AnimationChannel {
public:
    explicit AnimationChannel(ObjectId targetNode = kInvalidObjectId);
    ObjectId targetNode() const;
    std::vector<KeyframeVec3>& translations();
    std::vector<KeyframeVec4>& rotations();
    std::vector<KeyframeVec3>& scales();
    const std::vector<KeyframeVec3>& translations() const;
    const std::vector<KeyframeVec4>& rotations() const;
    const std::vector<KeyframeVec3>& scales() const;
    Transform sample(float time) const;

private:
    ObjectId targetNode_ = kInvalidObjectId;
    std::vector<KeyframeVec3> translations_;
    std::vector<KeyframeVec4> rotations_;
    std::vector<KeyframeVec3> scales_;
};

class AnimationClip {
public:
    AnimationClip() = default;
    AnimationClip(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    void setDuration(float duration);
    float duration() const;
    std::vector<AnimationChannel>& channels();
    const std::vector<AnimationChannel>& channels() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    float duration_ = 0.0f;
    std::vector<AnimationChannel> channels_;
};

struct Bone {
    ObjectId id = kInvalidObjectId;
    ObjectId parentId = kInvalidObjectId;
    std::string name;
    Transform bindPose;
    Mat4 inverseBindMatrix = Mat4::identity();
};

class Skeleton {
public:
    Skeleton() = default;
    Skeleton(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    std::vector<Bone>& bones();
    const std::vector<Bone>& bones() const;
    const Bone* findBone(ObjectId id) const;
    const Bone* findBoneByName(const std::string& name) const;
    void rebuildNameMap();

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    std::vector<Bone> bones_;
    std::unordered_map<std::string, std::size_t> nameToIndex_;
};

class Camera {
public:
    Camera() = default;
    Camera(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    void setPerspective(float fovYRadians, float nearPlane, float farPlane);
    float fovYRadians() const;
    float nearPlane() const;
    float farPlane() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    float fovYRadians_ = 1.04719755f;
    float nearPlane_ = 0.01f;
    float farPlane_ = 1000.0f;
};

enum class LightKind : std::uint32_t { Directional, Point, Spot };

class Light {
public:
    Light() = default;
    Light(ObjectId id, std::string name, LightKind kind);
    ObjectId id() const;
    const std::string& name() const;
    LightKind kind() const;
    void setColor(Vec3 color);
    Vec3 color() const;
    void setIntensity(float intensity);
    float intensity() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    LightKind kind_ = LightKind::Point;
    Vec3 color_{1.0f, 1.0f, 1.0f};
    float intensity_ = 1.0f;
};

class SceneNode {
public:
    SceneNode() = default;
    SceneNode(ObjectId id, std::string name);
    ObjectId id() const;
    const std::string& name() const;
    ObjectId parentId() const;
    void setParentId(ObjectId id);
    std::vector<ObjectId>& children();
    const std::vector<ObjectId>& children() const;
    Transform& localTransform();
    const Transform& localTransform() const;
    void setMeshId(ObjectId id);
    ObjectId meshId() const;
    void setSkeletonId(ObjectId id);
    ObjectId skeletonId() const;
    void setCameraId(ObjectId id);
    ObjectId cameraId() const;
    void setLightId(ObjectId id);
    ObjectId lightId() const;

private:
    ObjectId id_ = kInvalidObjectId;
    std::string name_;
    ObjectId parentId_ = kInvalidObjectId;
    std::vector<ObjectId> children_;
    Transform localTransform_;
    ObjectId meshId_ = kInvalidObjectId;
    ObjectId skeletonId_ = kInvalidObjectId;
    ObjectId cameraId_ = kInvalidObjectId;
    ObjectId lightId_ = kInvalidObjectId;
};

class Scene {
public:
    using NodeVisitor = std::function<void(const SceneNode&, const Mat4&)>;
    void clear();
    void setName(std::string name);
    const std::string& name() const;
    Mesh& addMesh(Mesh mesh);
    Material& addMaterial(Material material);
    TextureMetadata& addTexture(TextureMetadata texture);
    AnimationClip& addAnimation(AnimationClip animation);
    Skeleton& addSkeleton(Skeleton skeleton);
    Camera& addCamera(Camera camera);
    Light& addLight(Light light);
    SceneNode& addNode(SceneNode node);
    const Mesh* findMesh(ObjectId id) const;
    Mesh* findMesh(ObjectId id);
    const Material* findMaterial(ObjectId id) const;
    const TextureMetadata* findTexture(ObjectId id) const;
    const AnimationClip* findAnimation(ObjectId id) const;
    const Skeleton* findSkeleton(ObjectId id) const;
    const Camera* findCamera(ObjectId id) const;
    const Light* findLight(ObjectId id) const;
    const SceneNode* findNode(ObjectId id) const;
    SceneNode* findNode(ObjectId id);
    const std::vector<Mesh>& meshes() const;
    const std::vector<Material>& materials() const;
    const std::vector<TextureMetadata>& textures() const;
    const std::vector<AnimationClip>& animations() const;
    const std::vector<Skeleton>& skeletons() const;
    const std::vector<Camera>& cameras() const;
    const std::vector<Light>& lights() const;
    const std::vector<SceneNode>& nodes() const;
    std::vector<SceneNode>& nodes();
    std::vector<ObjectId> rootNodes() const;
    void rebuildIndexes();
    void traverse(const NodeVisitor& visitor) const;
    Mat4 worldTransform(ObjectId nodeId) const;

private:
    void traverseFrom(ObjectId nodeId, const Mat4& parent, const NodeVisitor& visitor) const;
    std::string name_ = "Untitled Scene";
    std::vector<Mesh> meshes_;
    std::vector<Material> materials_;
    std::vector<TextureMetadata> textures_;
    std::vector<AnimationClip> animations_;
    std::vector<Skeleton> skeletons_;
    std::vector<Camera> cameras_;
    std::vector<Light> lights_;
    std::vector<SceneNode> nodes_;
    std::unordered_map<ObjectId, std::size_t> meshIndex_;
    std::unordered_map<ObjectId, std::size_t> materialIndex_;
    std::unordered_map<ObjectId, std::size_t> textureIndex_;
    std::unordered_map<ObjectId, std::size_t> animationIndex_;
    std::unordered_map<ObjectId, std::size_t> skeletonIndex_;
    std::unordered_map<ObjectId, std::size_t> cameraIndex_;
    std::unordered_map<ObjectId, std::size_t> lightIndex_;
    std::unordered_map<ObjectId, std::size_t> nodeIndex_;
};

namespace compression {
std::vector<std::uint8_t> compressRle(const std::vector<std::uint8_t>& input);
Result<std::vector<std::uint8_t>> decompressRle(const std::vector<std::uint8_t>& input, std::size_t expectedSize);
bool shouldCompress(const std::vector<std::uint8_t>& input);
} // namespace compression

namespace parser {

struct ParserOptions {
    bool allowUnknownOptionalSections = true;
    std::size_t maximumSectionSize = 128u * 1024u * 1024u;
};

struct RawSection {
    format::SectionRecord record{};
    std::vector<std::uint8_t> data;
};

struct ParseContext {
    ParserOptions options;
    const std::uint8_t* input = nullptr;
    std::size_t inputSize = 0;
    format::FileHeader header{};
    std::vector<format::SectionRecord> sections;
    std::unordered_map<std::uint32_t, RawSection> sectionData;
    Scene scene;
};

class SceneParser {
public:
    explicit SceneParser(ParserOptions options = {});
    Result<Scene> parse(const std::vector<std::uint8_t>& bytes) const;
    Result<Scene> parse(const std::uint8_t* data, std::size_t size) const;

private:
    ParserOptions options_;
};

Result<void> parseHeader(ParseContext& context);
Result<void> validateVersion(ParseContext& context);
Result<void> discoverSections(ParseContext& context);
Result<void> validateOffsets(ParseContext& context);
Result<void> createObjects(ParseContext& context);
Result<void> resolveReferences(ParseContext& context);
Result<void> loadAnimations(ParseContext& context);
Result<void> constructScene(ParseContext& context);

} // namespace parser

namespace serialization {

struct SerializerOptions {
    bool compressLargeSections = true;
    bool includeChecksum = true;
    std::uint32_t alignment = 8;
};

class SceneSerializer {
public:
    explicit SceneSerializer(SerializerOptions options = {});
    Result<std::vector<std::uint8_t>> serialize(const Scene& scene) const;

private:
    SerializerOptions options_;
};

class SceneDeserializer {
public:
    explicit SceneDeserializer(parser::ParserOptions options = {});
    Result<Scene> deserialize(const std::vector<std::uint8_t>& bytes) const;

private:
    parser::SceneParser parser_;
};

} // namespace serialization

namespace resource {

struct CacheStats {
    std::size_t liveResources = 0;
    std::size_t totalLoads = 0;
    std::size_t cacheHits = 0;
    std::size_t evictions = 0;
};

template <typename T>
struct ResourceHandle {
    std::string key;
    std::shared_ptr<T> resource;
    explicit operator bool() const { return static_cast<bool>(resource); }
    T* operator->() { return resource.get(); }
    const T* operator->() const { return resource.get(); }
};

class ResourceCache {
public:
    template <typename T, typename Loader>
    ResourceHandle<T> load(const std::string& key, Loader loader) {
        auto found = entries_.find(key);
        if (found != entries_.end()) {
            ++stats_.cacheHits;
            return ResourceHandle<T>{key, std::static_pointer_cast<T>(found->second.value)};
        }
        auto value = std::make_shared<T>(loader());
        entries_[key] = Entry{value};
        ++stats_.totalLoads;
        stats_.liveResources = entries_.size();
        return ResourceHandle<T>{key, value};
    }
    bool contains(const std::string& key) const;
    void releaseUnused();
    void evict(const std::string& key);
    void clear();
    CacheStats stats() const;

private:
    struct Entry { std::shared_ptr<void> value; };
    std::unordered_map<std::string, Entry> entries_;
    CacheStats stats_;
};

} // namespace resource

namespace validation {

struct ValidationOptions {
    bool requireValidMeshIndices = true;
    bool requireExistingReferences = true;
    bool requireAcyclicHierarchy = true;
};

class SceneValidator {
public:
    explicit SceneValidator(ValidationOptions options = {});
    ErrorList validate(const Scene& scene) const;

private:
    ValidationOptions options_;
};

} // namespace validation

namespace util {
std::uint32_t fnv1a32(const std::vector<std::uint8_t>& bytes);
std::uint32_t fnv1a32(const std::uint8_t* bytes, std::size_t size);
std::string normalizePath(std::string path);
std::string basename(const std::string& path);
} // namespace util

} // namespace bse
