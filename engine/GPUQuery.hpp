#include <cstdint>
typedef unsigned int GLuint;

class GPUQuery {
public:
    void Begin();
    uint64_t End();
private:
    GLuint _query_object = 0;
};
