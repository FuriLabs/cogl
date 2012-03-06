#include <cogl/cogl2-experimental.h>

#include "test-utils.h"

/* This test assumes the GL driver supports point sizes up to 16
   pixels. Cogl should probably have some way of querying the size so
   we start from that instead */
#define MAX_POINT_SIZE 16
/* The size of the area that we'll paint each point in */
#define POINT_BOX_SIZE (MAX_POINT_SIZE * 2)

static int
calc_coord_offset (int pos, int pos_index, int point_size)
{
  switch (pos_index)
    {
    case 0: return pos - point_size / 2 - 2;
    case 1: return pos - point_size / 2 + 2;
    case 2: return pos + point_size / 2 - 2;
    case 3: return pos + point_size / 2 + 2;
    }

  g_assert_not_reached ();
}

static void
verify_point_size (CoglFramebuffer *fb,
                   int x_pos,
                   int y_pos,
                   int point_size)
{
  int y, x;

  for (y = 0; y < 4; y++)
    for (x = 0; x < 4; x++)
      {
        gboolean in_point = x >= 1 && x <= 2 && y >= 1 && y <= 2;
        guint32 expected_pixel = in_point ? 0x00ff00ff : 0xff0000ff;

        test_utils_check_pixel (calc_coord_offset (x_pos, x, point_size),
                                calc_coord_offset (y_pos, y, point_size),
                                expected_pixel);
      }
}

void
test_cogl_point_size (TestUtilsGTestFixture *fixture,
                      void *data)
{
  TestUtilsSharedState *shared_state = data;
  CoglContext *ctx = shared_state->ctx;
  int fb_width = cogl_framebuffer_get_width (shared_state->fb);
  int fb_height = cogl_framebuffer_get_height (shared_state->fb);
  int point_size;
  int x_pos;

  cogl_framebuffer_orthographic (shared_state->fb,
                                 0, 0, /* x_1, y_1 */
                                 fb_width, /* x_2 */
                                 fb_height /* y_2 */,
                                 -1, 100 /* near/far */);

  cogl_framebuffer_clear4f (shared_state->fb,
                            COGL_BUFFER_BIT_COLOR,
                            1.0f, 0.0f, 0.0f, 1.0f);

  /* Try a rendering a single point with a few different point
     sizes */
  for (x_pos = 0, point_size = MAX_POINT_SIZE;
       point_size >= 4;
       x_pos += POINT_BOX_SIZE, point_size /= 2)
    {
      CoglPipeline *pipeline = cogl_pipeline_new (ctx);
      CoglVertexP2 point = { x_pos + POINT_BOX_SIZE / 2,
                             POINT_BOX_SIZE / 2 };
      CoglPrimitive *prim =
        cogl_primitive_new_p2 (ctx,
                               COGL_VERTICES_MODE_POINTS,
                               1, /* n_vertices */
                               &point);

      cogl_pipeline_set_point_size (pipeline, point_size);
      cogl_pipeline_set_color4ub (pipeline, 0, 255, 0, 255);
      cogl_framebuffer_draw_primitive (shared_state->fb,
                                       pipeline,
                                       prim);

      cogl_object_unref (prim);
      cogl_object_unref (pipeline);
    }

  /* Verify all of the points where drawn at the right size */
  for (x_pos = 0, point_size = MAX_POINT_SIZE;
       point_size >= 4;
       x_pos += POINT_BOX_SIZE, point_size /= 2)
    verify_point_size (shared_state->fb,
                       x_pos + POINT_BOX_SIZE / 2,
                       POINT_BOX_SIZE / 2,
                       point_size);

  if (cogl_test_verbose ())
    g_print ("OK\n");
}
