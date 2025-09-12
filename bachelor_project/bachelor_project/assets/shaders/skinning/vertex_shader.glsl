#version 450

layout(push_constant) uniform SkinningPush {
    uint dynamic_positions_offset;
	uint joint_matrices_offset;
	uint joint_weights_offset;
	uint joint_weights_per_vertex;
} skinning_push;

layout(location = 0) in vec3 in_pos;

struct Pos {
    float x;
    float y;
    float z;
};

struct JointWeight {
    uint index;
    float weight;
};

layout(std430, set = 0, binding = 0) buffer DynPositions {
    Pos dyn_positions[];
} dyn_positions;

layout(std430, set = 0, binding = 1) readonly buffer Joints {
    mat4 joint_mats[];
} joints;

layout(std430, set = 0, binding = 2) readonly buffer JointWeights {
    JointWeight joint_weights[];
} joint_weights;


void main() {
    mat4 transform_mat = mat4(0.0);
    for(uint i = 0; i < skinning_push.joint_weights_per_vertex; i++) {
        uint weight_index = skinning_push.joint_weights_offset + skinning_push.joint_weights_per_vertex * gl_VertexIndex + i;
        transform_mat += joint_weights.joint_weights[weight_index].weight * joints.joint_mats[skinning_push.joint_matrices_offset + joint_weights.joint_weights[weight_index].index];
    }

    vec4 transformed_pos = transform_mat * vec4(in_pos, 1.0);

    dyn_positions.dyn_positions[skinning_push.dynamic_positions_offset + gl_VertexIndex].x = transformed_pos.x;
    dyn_positions.dyn_positions[skinning_push.dynamic_positions_offset + gl_VertexIndex].y = transformed_pos.y;
    dyn_positions.dyn_positions[skinning_push.dynamic_positions_offset + gl_VertexIndex].z = transformed_pos.z;

    gl_PointSize = 1.0;
}