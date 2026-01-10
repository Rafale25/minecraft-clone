#include <cstdint>

typedef unsigned int GLuint;

class GPUQuery {
public:
    void Begin();
    uint64_t End();
private:
    GLuint m_queryObject = 0;
};
