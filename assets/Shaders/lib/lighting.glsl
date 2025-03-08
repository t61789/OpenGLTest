#define MAX_PARALLEL_LIGHT_COUNT 4
#define MAX_POINT_LIGHT_COUNT 16

struct ParallelLightInfo
{
    vec3 direction;
    vec3 color;
};
// size 6
uniform float _ParallelLightInfo[6 * MAX_PARALLEL_LIGHT_COUNT];
uniform int _ParallelLightCount;

struct PointLightInfo
{
    vec3 positionWS;
    float radius;
    vec3 color;
};
// size 7
uniform float _PointLightInfo[7 * MAX_POINT_LIGHT_COUNT];
uniform int _PointLightCount;
