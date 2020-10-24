/*************************************************************************/
/*  rasterizer_canvas_rd.h                                               */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef RASTERIZER_CANVAS_RD_H
#define RASTERIZER_CANVAS_RD_H

#include "servers/rendering/rasterizer.h"
#include "servers/rendering/rasterizer_rd/rasterizer_storage_rd.h"
#include "servers/rendering/rasterizer_rd/render_pipeline_vertex_format_cache_rd.h"
#include "servers/rendering/rasterizer_rd/shader_compiler_rd.h"
#include "servers/rendering/rasterizer_rd/shaders/canvas.glsl.gen.h"
#include "servers/rendering/rasterizer_rd/shaders/canvas_occlusion.glsl.gen.h"
#include "servers/rendering/rendering_device.h"

class RasterizerCanvasRD : public RasterizerCanvas {
	RasterizerStorageRD *storage;

	enum {
		BASE_UNIFORM_SET = 0,
		MATERIAL_UNIFORM_SET = 1,
		TRANSFORMS_UNIFORM_SET = 2,
		CANVAS_TEXTURE_UNIFORM_SET = 3,
	};

	enum ShaderVariant {
		SHADER_VARIANT_QUAD,
		SHADER_VARIANT_NINEPATCH,
		SHADER_VARIANT_PRIMITIVE,
		SHADER_VARIANT_PRIMITIVE_POINTS,
		SHADER_VARIANT_ATTRIBUTES,
		SHADER_VARIANT_ATTRIBUTES_POINTS,
		SHADER_VARIANT_QUAD_LIGHT,
		SHADER_VARIANT_NINEPATCH_LIGHT,
		SHADER_VARIANT_PRIMITIVE_LIGHT,
		SHADER_VARIANT_PRIMITIVE_POINTS_LIGHT,
		SHADER_VARIANT_ATTRIBUTES_LIGHT,
		SHADER_VARIANT_ATTRIBUTES_POINTS_LIGHT,
		SHADER_VARIANT_MAX
	};

	enum {
		FLAGS_INSTANCING_STRIDE_MASK = 0xF,
		FLAGS_INSTANCING_ENABLED = (1 << 4),
		FLAGS_INSTANCING_HAS_COLORS = (1 << 5),
		FLAGS_INSTANCING_COLOR_8BIT = (1 << 6),
		FLAGS_INSTANCING_HAS_CUSTOM_DATA = (1 << 7),
		FLAGS_INSTANCING_CUSTOM_DATA_8_BIT = (1 << 8),

		FLAGS_CLIP_RECT_UV = (1 << 9),
		FLAGS_TRANSPOSE_RECT = (1 << 10),
		FLAGS_USING_LIGHT_MASK = (1 << 11),

		FLAGS_NINEPACH_DRAW_CENTER = (1 << 12),
		FLAGS_USING_PARTICLES = (1 << 13),
		FLAGS_USE_PIXEL_SNAP = (1 << 14),

		FLAGS_USE_SKELETON = (1 << 15),
		FLAGS_NINEPATCH_H_MODE_SHIFT = 16,
		FLAGS_NINEPATCH_V_MODE_SHIFT = 18,
		FLAGS_LIGHT_COUNT_SHIFT = 20,

		FLAGS_DEFAULT_NORMAL_MAP_USED = (1 << 26),
		FLAGS_DEFAULT_SPECULAR_MAP_USED = (1 << 27)

	};

	enum {
		LIGHT_FLAGS_TEXTURE_MASK = 0xFFFF,
		LIGHT_FLAGS_BLEND_SHIFT = 16,
		LIGHT_FLAGS_BLEND_MASK = (3 << 16),
		LIGHT_FLAGS_BLEND_MODE_ADD = (0 << 16),
		LIGHT_FLAGS_BLEND_MODE_SUB = (1 << 16),
		LIGHT_FLAGS_BLEND_MODE_MIX = (2 << 16),
		LIGHT_FLAGS_BLEND_MODE_MASK = (3 << 16),
		LIGHT_FLAGS_HAS_SHADOW = (1 << 20),
		LIGHT_FLAGS_FILTER_SHIFT = 22

	};

	enum {
		MAX_RENDER_ITEMS = 256 * 1024,
		MAX_LIGHT_TEXTURES = 1024,
		MAX_LIGHTS_PER_ITEM = 16,
		DEFAULT_MAX_LIGHTS_PER_RENDER = 256
	};

	/****************/
	/**** SHADER ****/
	/****************/

	enum PipelineVariant {
		PIPELINE_VARIANT_QUAD,
		PIPELINE_VARIANT_NINEPATCH,
		PIPELINE_VARIANT_PRIMITIVE_TRIANGLES,
		PIPELINE_VARIANT_PRIMITIVE_LINES,
		PIPELINE_VARIANT_PRIMITIVE_POINTS,
		PIPELINE_VARIANT_ATTRIBUTE_TRIANGLES,
		PIPELINE_VARIANT_ATTRIBUTE_TRIANGLE_STRIP,
		PIPELINE_VARIANT_ATTRIBUTE_LINES,
		PIPELINE_VARIANT_ATTRIBUTE_LINES_STRIP,
		PIPELINE_VARIANT_ATTRIBUTE_POINTS,
		PIPELINE_VARIANT_MAX
	};
	enum PipelineLightMode {
		PIPELINE_LIGHT_MODE_DISABLED,
		PIPELINE_LIGHT_MODE_ENABLED,
		PIPELINE_LIGHT_MODE_MAX
	};

	struct PipelineVariants {
		RenderPipelineVertexFormatCacheRD variants[PIPELINE_LIGHT_MODE_MAX][PIPELINE_VARIANT_MAX];
	};

	struct {
		CanvasShaderRD canvas_shader;
		RID default_version;
		RID default_version_rd_shader;
		RID quad_index_buffer;
		RID quad_index_array;
		PipelineVariants pipeline_variants;

		// default_skeleton uniform set
		RID default_skeleton_uniform_buffer;
		RID default_skeleton_texture_buffer;

		ShaderCompilerRD compiler;
	} shader;

	struct ShaderData : public RasterizerStorageRD::ShaderData {
		enum BlendMode { //used internally
			BLEND_MODE_MIX,
			BLEND_MODE_ADD,
			BLEND_MODE_SUB,
			BLEND_MODE_MUL,
			BLEND_MODE_PMALPHA,
			BLEND_MODE_DISABLED,
		};

		enum LightMode {
			LIGHT_MODE_NORMAL,
			LIGHT_MODE_UNSHADED,
			LIGHT_MODE_LIGHT_ONLY
		};

		bool valid;
		RID version;
		PipelineVariants pipeline_variants;
		String path;

		Map<StringName, ShaderLanguage::ShaderNode::Uniform> uniforms;
		Vector<ShaderCompilerRD::GeneratedCode::Texture> texture_uniforms;

		Vector<uint32_t> ubo_offsets;
		uint32_t ubo_size;

		String code;
		Map<StringName, RID> default_texture_params;

		bool uses_screen_texture;

		virtual void set_code(const String &p_Code);
		virtual void set_default_texture_param(const StringName &p_name, RID p_texture);
		virtual void get_param_list(List<PropertyInfo> *p_param_list) const;
		virtual void get_instance_param_list(List<RasterizerStorage::InstanceShaderParam> *p_param_list) const;

		virtual bool is_param_texture(const StringName &p_param) const;
		virtual bool is_animated() const;
		virtual bool casts_shadows() const;
		virtual Variant get_default_parameter(const StringName &p_parameter) const;
		ShaderData();
		virtual ~ShaderData();
	};

	RasterizerStorageRD::ShaderData *_create_shader_func();
	static RasterizerStorageRD::ShaderData *_create_shader_funcs() {
		return static_cast<RasterizerCanvasRD *>(singleton)->_create_shader_func();
	}

	struct MaterialData : public RasterizerStorageRD::MaterialData {
		uint64_t last_frame;
		ShaderData *shader_data;
		RID uniform_buffer;
		RID uniform_set;
		Vector<RID> texture_cache;
		Vector<uint8_t> ubo_data;

		virtual void set_render_priority(int p_priority) {}
		virtual void set_next_pass(RID p_pass) {}
		virtual void update_parameters(const Map<StringName, Variant> &p_parameters, bool p_uniform_dirty, bool p_textures_dirty);
		virtual ~MaterialData();
	};

	RasterizerStorageRD::MaterialData *_create_material_func(ShaderData *p_shader);
	static RasterizerStorageRD::MaterialData *_create_material_funcs(RasterizerStorageRD::ShaderData *p_shader) {
		return static_cast<RasterizerCanvasRD *>(singleton)->_create_material_func(static_cast<ShaderData *>(p_shader));
	}

	/**************************/
	/**** CANVAS TEXTURES *****/
	/**************************/

	struct {
		RS::CanvasItemTextureFilter default_filter;
		RS::CanvasItemTextureRepeat default_repeat;
	} default_samplers;

	/******************/
	/**** POLYGONS ****/
	/******************/

	struct PolygonBuffers {
		RD::VertexFormatID vertex_format_id;
		RID vertex_buffer;
		RID vertex_array;
		RID index_buffer;
		RID indices;
	};

	struct {
		HashMap<PolygonID, PolygonBuffers> polygons;
		PolygonID last_id;
	} polygon_buffers;

	/********************/
	/**** PRIMITIVES ****/
	/********************/

	struct {
		RID index_array[4];
	} primitive_arrays;

	/*******************/
	/**** MATERIALS ****/
	/*******************/

	/******************/
	/**** LIGHTING ****/
	/******************/

	struct CanvasLight {
		RID texture;
		struct {
			bool enabled = false;
			float z_far;
			float y_offset;
		} shadow;
	};

	RID_Owner<CanvasLight> canvas_light_owner;

	struct ShadowRenderPushConstant {
		float projection[16];
		float modelview[8];
		float direction[2];
		float z_far;
		float pad;
	};

	struct OccluderPolygon {
		RS::CanvasOccluderPolygonCullMode cull_mode;
		int point_count;
		RID vertex_buffer;
		RID vertex_array;
		RID index_buffer;
		RID index_array;
	};

	struct LightUniform {
		float matrix[8]; //light to texture coordinate matrix
		float shadow_matrix[8]; //light to shadow coordinate matrix
		float color[4];

		uint8_t shadow_color[4];
		uint32_t flags; //index to light texture
		float shadow_pixel_size;
		float height;

		float position[2];
		float shadow_z_far_inv;
		float shadow_y_ofs;

		float atlas_rect[4];
	};

	RID_Owner<OccluderPolygon> occluder_polygon_owner;

	struct {
		CanvasOcclusionShaderRD shader;
		RID shader_version;
		RID render_pipelines[3];
		RD::VertexFormatID vertex_format;
		RD::FramebufferFormatID framebuffer_format;
	} shadow_render;

	/***************/
	/**** STATE ****/
	/***************/

	//state that does not vary across rendering all items

	struct State {
		//state buffer
		struct Buffer {
			float canvas_transform[16];
			float screen_transform[16];
			float canvas_normal_transform[16];
			float canvas_modulate[4];
			float screen_pixel_size[2];
			float time;
			float pad;

			//uint32_t light_count;
			//uint32_t pad[3];
		};

		LightUniform *light_uniforms;

		RID lights_uniform_buffer;
		RID canvas_state_buffer;
		RID shadow_sampler;
		RID shadow_texture;
		RID shadow_depth_texture;
		RID shadow_fb;
		int shadow_texture_size = 2048;

		RID default_transforms_uniform_set;

		uint32_t max_lights_per_render;
		uint32_t max_lights_per_item;

		double time;
	} state;

	struct PushConstant {
		float world[6];
		uint32_t flags;
		uint32_t specular_shininess;
		union {
			//rect
			struct {
				float modulation[4];
				float ninepatch_margins[4];
				float dst_rect[4];
				float src_rect[4];
				float pad[2];
			};
			//primitive
			struct {
				float points[6]; // vec2 points[3]
				float uvs[6]; // vec2 points[3]
				uint32_t colors[6]; // colors encoded as half
			};
		};
		float color_texture_pixel_size[2];
		uint32_t lights[4];
	};

	struct SkeletonUniform {
		float skeleton_transform[16];
		float skeleton_inverse[16];
	};

	Item *items[MAX_RENDER_ITEMS];

	RID default_canvas_texture;

	RS::CanvasItemTextureFilter default_filter = RS::CANVAS_ITEM_TEXTURE_FILTER_LINEAR;
	RS::CanvasItemTextureRepeat default_repeat = RS::CANVAS_ITEM_TEXTURE_REPEAT_DISABLED;

	RID _create_base_uniform_set(RID p_to_render_target, bool p_backbuffer);

	inline void _bind_canvas_texture(RD::DrawListID p_draw_list, RID p_texture, RS::CanvasItemTextureFilter p_base_filter, RS::CanvasItemTextureRepeat p_base_repeat, RID &r_last_texture, PushConstant &push_constant, Size2 &r_texpixel_size); //recursive, so regular inline used instead.
	void _render_item(RenderingDevice::DrawListID p_draw_list, const Item *p_item, RenderingDevice::FramebufferFormatID p_framebuffer_format, const Transform2D &p_canvas_transform_inverse, Item *&current_clip, Light *p_lights, PipelineVariants *p_pipeline_variants);
	void _render_items(RID p_to_render_target, int p_item_count, const Transform2D &p_canvas_transform_inverse, Light *p_lights);

	_FORCE_INLINE_ void _update_transform_2d_to_mat2x4(const Transform2D &p_transform, float *p_mat2x4);
	_FORCE_INLINE_ void _update_transform_2d_to_mat2x3(const Transform2D &p_transform, float *p_mat2x3);

	_FORCE_INLINE_ void _update_transform_2d_to_mat4(const Transform2D &p_transform, float *p_mat4);
	_FORCE_INLINE_ void _update_transform_to_mat4(const Transform &p_transform, float *p_mat4);

public:
	PolygonID request_polygon(const Vector<int> &p_indices, const Vector<Point2> &p_points, const Vector<Color> &p_colors, const Vector<Point2> &p_uvs = Vector<Point2>(), const Vector<int> &p_bones = Vector<int>(), const Vector<float> &p_weights = Vector<float>());
	void free_polygon(PolygonID p_polygon);

	RID light_create();
	void light_set_texture(RID p_rid, RID p_texture);
	void light_set_use_shadow(RID p_rid, bool p_enable);
	void light_update_shadow(RID p_rid, int p_shadow_index, const Transform2D &p_light_xform, int p_light_mask, float p_near, float p_far, LightOccluderInstance *p_occluders);

	RID occluder_polygon_create();
	void occluder_polygon_set_shape_as_lines(RID p_occluder, const Vector<Vector2> &p_lines);
	void occluder_polygon_set_cull_mode(RID p_occluder, RS::CanvasOccluderPolygonCullMode p_mode);

	void canvas_render_items(RID p_to_render_target, Item *p_item_list, const Color &p_modulate, Light *p_light_list, const Transform2D &p_canvas_transform, RS::CanvasItemTextureFilter p_default_filter, RS::CanvasItemTextureRepeat p_default_repeat);

	void canvas_debug_viewport_shadows(Light *p_lights_with_shadow) {}

	void draw_window_margins(int *p_margins, RID *p_margin_textures) {}

	virtual void set_shadow_texture_size(int p_size);

	void set_time(double p_time);
	void update();
	bool free(RID p_rid);
	RasterizerCanvasRD(RasterizerStorageRD *p_storage);
	~RasterizerCanvasRD();
};

#endif // RASTERIZER_CANVAS_RD_H