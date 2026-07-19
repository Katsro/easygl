#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <unordered_map>

namespace easygl {
    class context;
    constexpr auto VERT = GL_VERTEX_SHADER;
    constexpr auto COMP = GL_COMPUTE_SHADER;
    constexpr auto FRAG = GL_FRAGMENT_SHADER;
    constexpr auto SDIB = GL_DRAW_INDIRECT_BUFFER;
    constexpr auto SCIB = GL_DISPATCH_INDIRECT_BUFFER;
    constexpr auto READ = GL_READ_ONLY;
    constexpr auto WRITE = GL_WRITE_ONLY;
    constexpr auto RGBA8 = GL_RGBA8;
    constexpr auto RGBA16F = GL_RGBA16F;
    constexpr auto RGBA32F = GL_RGBA32F;
    constexpr auto RGBA16I = GL_RGBA16I;
    constexpr auto RGBA32I = GL_RGBA32I;
    constexpr auto RGBA16UI = GL_RGBA16UI;
    constexpr auto RGBA32UI = GL_RGBA32UI;
    constexpr auto LINE = GL_LINE;
    constexpr auto FAN = GL_TRIANGLE_FAN;
    constexpr auto STRIP = GL_TRIANGLE_STRIP;

    using str = std::string;
    using cxtcall = void(&)(context &cxt);

    float time();

    void start(const str &name, cxtcall create, cxtcall update,
               bool full = false, int w = 900, int h = 600);
}

class easygl::context {
    struct ment {
        GLuint uuid;
        GLenum type;
    };

    std::unordered_map<str, ment> imges{};
    std::unordered_map<str, ment> buffs{};
    std::unordered_map<str, ment> progs{};
    std::unordered_map<str, ment> pipes{};

public:
    GLFWwindow *window = nullptr;

    void imge(const str &name, GLsizei w, GLsizei h, GLenum type) {
        auto &[id,ty] = imges[name] = {0, type};
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexStorage2D(GL_TEXTURE_2D, 1, ty, w, h);
    }

    void buff(const str &name, GLsizei size, GLenum type) {
        auto &[id,ty] = buffs[name] = {0, type};
        glGenBuffers(1, &id);
        glBufferData(ty, size, nullptr, GL_DYNAMIC_DRAW);
    }

    void prog(const str &name, GLenum type, const std::filesystem::path &file) {
        // ① try the path as-is
        std::filesystem::path resolved = file;
        if (!std::filesystem::exists(resolved)) {
            // ② fallback: prepend EASYGL_ROOT (CMake preset macro)
#ifdef EASYGL_ROOT
            std::filesystem::path alt = std::filesystem::path(EASYGL_ROOT) / file;
            if (std::filesystem::exists(alt)) resolved = alt;
#endif
        }

        const auto size = std::filesystem::file_size(resolved);
        std::string buf(size, '\0');
        std::ifstream(resolved).read(buf.data(), static_cast<long long>(size));
        progSrc(name, type, buf);
    }

    // Like prog() but takes GLSL source directly — no file needed.
    void progSrc(const str &name, GLenum type, const str &source) {
        auto &[id,ty] = progs[name] = {0, type};
        const GLchar *src = source.data();
        id = glCreateShaderProgramv(ty, 1, &src);
        GLint done = GL_FALSE;
        glGetProgramiv(id, GL_LINK_STATUS, &done);
        if (done == GL_FALSE) {
            GLsizei len = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
            std::string log(1llu + len, '\0');
            glGetProgramInfoLog(id, len, &len, log.data());
            throw std::runtime_error(name + log);
        }
    }

    void pipe(const str &name, const str &vert, const str &frag) {
        auto &[id,ty] = pipes[name] = {0, 0};
        glCreateProgramPipelines(1, &id);
        glUseProgramStages(id,GL_VERTEX_SHADER_BIT, progs[vert].uuid);
        glUseProgramStages(id,GL_FRAGMENT_SHADER_BIT, progs[frag].uuid);
        glValidateProgramPipeline(id);
        GLint done = GL_FALSE;
        glGetProgramPipelineiv(id, GL_VALIDATE_STATUS, &done);
        if (done == GL_FALSE) {
            GLsizei len = 0;
            glGetProgramPipelineiv(id, GL_INFO_LOG_LENGTH, &len);
            std::string log(1llu + len, '\0');
            glGetProgramPipelineInfoLog(id, len, &len, log.data());
            throw std::runtime_error(name + log);
        }
    }

    void setBuff(const str &name, GLsizeiptr offs, GLsizei size, const void *data) {
        auto &[id,ty] = buffs[name];
        glBindBuffer(ty, id);
        glBufferSubData(ty, offs, size, data);
    }

    void useImge(const str &name, GLuint binding, GLenum acce) {
        auto &[id,ty] = imges[name];
        glBindImageTexture(binding, id, 0, false, 0, acce, ty);
    }

    void useBuff(const str &name, GLuint binding) {
        auto &[id,ty] = buffs[name];
        glBindBufferBase(ty, binding, id);
    }

    void useComp(const str &name, GLuint x, GLuint y, GLuint z) {
        auto &[id,ty] = progs[name];
        glUseProgram(id);
        glDispatchCompute(x, y, z);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void useDraw(const str &name, GLenum mode, GLint fv, GLsizei vc, GLsizei ic) {
        auto &[id,ty] = pipes[name];
        glUseProgram(0);
        glBindProgramPipeline(id);
        glDrawArraysInstanced(mode, fv, vc, ic);
    }

    void indComp(const str &name, const str &buff) {
        auto &[id,ty] = progs[name];
        auto &[bf,_] = buffs[buff];
        glUseProgram(id);
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, bf);
        glDispatchComputeIndirect(0);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    void indDraw(const str &name, GLenum mode, const str &buff) {
        auto &[id,ty] = pipes[name];
        auto &[bf,_] = buffs[buff];
        glBindProgramPipeline(id);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, bf);
        glDrawArraysIndirect(mode, nullptr);
    }

    template<typename T>
    void uniform(const str &prog, const str &name, T data) noexcept = delete;

    // -- scalar-expanded overloads (no GLM required) --
    void uniform(const str &prog, const str &name, float x, float y) noexcept;

    void uniform(const str &prog, const str &name, float x, float y, float z) noexcept;

    void uniform(const str &prog, const str &name, float x, float y, float z, float w) noexcept;

    void uniform(const str &prog, const str &name, int x, int y) noexcept;

    void uniform(const str &prog, const str &name, int x, int y, int z) noexcept;

    void uniform(const str &prog, const str &name, int x, int y, int z, int w) noexcept;

    void uniform(const str &prog, const str &name, const float *m) noexcept;
};

inline void easygl::start(const str &name, cxtcall create, cxtcall update, bool full, int w, int h) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    context ctx;
    if (full) {
        const auto monitor = glfwGetPrimaryMonitor();
        const auto mode = glfwGetVideoMode(monitor);
        ctx.window = glfwCreateWindow(mode->width, mode->height, name.c_str(), monitor, nullptr);
    } else {
        ctx.window = glfwCreateWindow(w, h, name.c_str(), nullptr, nullptr);
    }
    if (!ctx.window) {
        glfwTerminate();
        throw std::runtime_error("glfwCreateWindow failed");
    }
    glfwMakeContextCurrent(ctx.window);
    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwTerminate();
        throw std::runtime_error("gladLoadGL failed");
    }
    glfwSetWindowSizeCallback(ctx.window, [](GLFWwindow *, int nw, int nh) {
        glViewport(0, 0, nw, nh);
    });
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glfwShowWindow(ctx.window);
    create(ctx);
    while (!glfwWindowShouldClose(ctx.window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        update(ctx);
        glfwPollEvents();
        glfwSwapBuffers(ctx.window);
    }
    glfwTerminate();
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, int data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1i(id, loc, data);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, float data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1f(id, loc, data);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, GLuint data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1ui(id, loc, data);
}

// -- scalar-expanded vec / mat4 overloads (no GLM dependency) --

inline void easygl::context::uniform(const str &prog, const str &name,
                                     float x, float y) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform2f(id, glGetUniformLocation(id, name.c_str()), x, y);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     float x, float y, float z) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform3f(id, glGetUniformLocation(id, name.c_str()), x, y, z);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     float x, float y, float z, float w) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform4f(id, glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     int x, int y) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform2i(id, glGetUniformLocation(id, name.c_str()), x, y);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     int x, int y, int z) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform3i(id, glGetUniformLocation(id, name.c_str()), x, y, z);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     int x, int y, int z, int w) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniform4i(id, glGetUniformLocation(id, name.c_str()), x, y, z, w);
}

inline void easygl::context::uniform(const str &prog, const str &name,
                                     const float *m) noexcept {
    const auto [id,ty] = progs[prog];
    glProgramUniformMatrix4fv(id, glGetUniformLocation(id, name.c_str()),
                              1, false, m);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, std::initializer_list<int> data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1iv(id, loc, static_cast<GLsizei>(data.size()), data.begin());
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, std::initializer_list<float> data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1fv(id, loc, static_cast<GLsizei>(data.size()), data.begin());
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, std::initializer_list<unsigned> data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform1uiv(id, loc, static_cast<GLsizei>(data.size()), data.begin());
}

// ── Optional GLM support ──
// Define EASYGL_HAVE_GLM (or set -DEASYGL_USE_GLM=ON in CMake) to
// re-enable glm::vec2/3/4, glm::ivec2/3/4, and glm::mat4 overloads.
#ifdef EASYGL_HAVE_GLM
#include <glm/glm.hpp>

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::vec2 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform2f(id, loc, data.x, data.y);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::vec3 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform3f(id, loc, data.x, data.y, data.z);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::vec4 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform4f(id, loc, data.x, data.y, data.z, data.w);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::ivec2 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform2i(id, loc, data.x, data.y);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::ivec3 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform3i(id, loc, data.x, data.y, data.z);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::ivec4 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniform4i(id, loc, data.x, data.y, data.z, data.w);
}

template<>
inline void easygl::context::uniform(const str &prog, const str &name, glm::mat4 data) noexcept {
    const auto [id,ty] = progs[prog];
    const auto loc = glGetUniformLocation(id, name.c_str());
    glProgramUniformMatrix4fv(id, loc, 1, false, &data[0][0]);
}
#endif
