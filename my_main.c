/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser,
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  push: push a new origin matrix onto the origin stack

  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix
                     based on the provided values, then
                     multiply the current top of the
                     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a
                    temporary matrix, multiply it by the
                    current top of the origins stack, then
                    call draw_polygons.

  line: create a line based on the provided values. Store
        that in a temporary matrix, multiply it by the
        current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the screen
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"
#include "gmath.h"

void my_main() {

  int i;
  struct matrix *tmp;
  struct matrix *polygons;
  struct matrix *edges;
  struct stack *systems;
  screen t;
  zbuffer zb;
  color g;
  double step_3d = 20;
  double theta;

  //Lighting values here for easy access
  color ambient;
  ambient.red = 50;
  ambient.green = 50;
  ambient.blue = 50;

  double light[2][3];
  light[LOCATION][0] = 0.5;
  light[LOCATION][1] = 0.75;
  light[LOCATION][2] = 1;

  light[COLOR][RED] = 0;
  light[COLOR][GREEN] = 255;
  light[COLOR][BLUE] = 255;

  double view[3];
  view[0] = 0;
  view[1] = 0;
  view[2] = 1;

  //default reflective constants if none are set in script file
  struct constants white;
  white.r[AMBIENT_R] = 0.1;
  white.g[AMBIENT_R] = 0.1;
  white.b[AMBIENT_R] = 0.1;

  white.r[DIFFUSE_R] = 0.5;
  white.g[DIFFUSE_R] = 0.5;
  white.b[DIFFUSE_R] = 0.5;

  white.r[SPECULAR_R] = 0.5;
  white.g[SPECULAR_R] = 0.5;
  white.b[SPECULAR_R] = 0.5;

  //constants are a pointer in symtab, using one here for consistency
  struct constants *reflect;
  reflect = &white;

  systems = new_stack();
  tmp = new_matrix(4, 1000);
  clear_screen( t );
  clear_zbuffer(zb);
  g.red = 0;
  g.green = 0;
  g.blue = 0;

  print_symtab();
  for (i=0;i<lastop;i++) {

    //printf("%d: ",i);

    //printf("\n");

    switch (op[i].opcode)
        {
        case LIGHT:

          light[COLOR][RED] = op[i].op.light.c[RED];
          light[COLOR][GREEN] = op[i].op.light.c[GREEN];
          light[COLOR][BLUE] = op[i].op.light.c[BLUE];

          break;
        case AMBIENT:
          ambient.red = op[i].op.ambient.c[RED];
          ambient.green = op[i].op.ambient.c[GREEN];
          ambient.blue = op[i].op.ambient.c[BLUE];
          break;

        case SAVE_COORDS:
          printf("WIP: Save coords\n");
          break;

        case CAMERA:
          printf("WIP: Camera\n");
          break;

        case SPHERE:
          if(op[i].op.sphere.constants != NULL){
            reflect = op[i].op.sphere.constants->s.c;
          } else {
            reflect = &white;
          }
          add_sphere(polygons, op[i].op.sphere.d[0], op[i].op.sphere.d[1], op[i].op.sphere.d[2], op[i].op.sphere.r, step_3d);
          matrix_mult(peek(systems),polygons);
          draw_polygons(polygons, t, zb, view, light, ambient, reflect);
          polygons->lastcol = 0;
          break;

        case TORUS:
          if(op[i].op.torus.constants != NULL){
            reflect = op[i].op.torus.constants->s.c;
          } else {
            reflect = &white;
          }
          add_torus(polygons, op[i].op.torus.d[0], op[i].op.torus.d[1], op[i].op.torus.d[2], op[i].op.torus.r0, op[i].op.torus.r1, step_3d);
          matrix_mult(peek(systems),polygons);
          draw_polygons(polygons, t, zb, view, light, ambient, reflect);
          polygons->lastcol = 0;
          break;

        case BOX:
          if(op[i].op.box.constants != NULL){
            reflect = op[i].op.box.constants->s.c;
          } else {
            reflect = &white;
          }
          add_box(polygons, op[i].op.box.d0[0], op[i].op.box.d0[1], op[i].op.box.d0[2], op[i].op.box.d1[0], op[i].op.box.d1[1], op[i].op.box.d1[2]);
          matrix_mult(peek(systems),polygons);
          draw_polygons(polygons, t, zb, view, light, ambient, reflect);
          polygons->lastcol = 0;
          break;

        case LINE:
          add_edge(edges, op[i].op.line.p0[0], op[i].op.line.p0[1], op[i].op.line.p0[2], op[i].op.line.p1[0], op[i].op.line.p1[1], op[i].op.line.p1[2]);
          matrix_mult(peek(systems),edges);
          draw_lines(edges, t, zb, g);
          edges->lastcol = 0;
          break;

        case MESH:
          printf("WIP: Mesh\n");
          break;

        case SET:
          printf("WIP: Set\n");
          break;

        case MOVE:
          tmp = make_translate(op[i].op.move.d[0], op[i].op.move.d[1], op[i].op.move.d[2]);
          matrix_mult(peek(systems),tmp);
          copy_matrix(tmp, peek(systems));
           free_matrix(tmp);
          break;

        case SCALE:
          tmp = make_scale(op[i].op.scale.d[0], op[i].op.scale.d[1], op[i].op.scale.d[2]);
          matrix_mult(peek(systems),tmp);
          copy_matrix(tmp, peek(systems));
          free_matrix(tmp);
          break;

        case ROTATE:
          op[i].op.rotate.degrees *= (M_PI / 180);
          if(op[i].op.rotate.axis == 0) tmp = make_rotX(op[i].op.rotate.degrees);
          else if(op[i].op.rotate.axis == 1) tmp = make_rotY(op[i].op.rotate.degrees);
          else if(op[i].op.rotate.axis == 2) tmp = make_rotZ(op[i].op.rotate.degrees);
          matrix_mult(peek(systems), tmp);
          copy_matrix(tmp, peek(systems));
          free_matrix(tmp);
          break;
          
        case BASENAME:
          printf("WIP: Basename\n");
          break;
        case SAVE_KNOBS:
          printf("WIP: Save knobs\n");
          break;
        case TWEEN:
          printf("WIP: Tween\n");
          break;
        case FRAMES:
          printf("WIP: Frames\n");
          break;
        case VARY:
          printf("WIP: Vary\n");
          break;
        case PUSH:
          
          push(systems);

          break;
        case POP:
          
          pop(systems);

          break;
        case GENERATE_RAYFILES:
          printf("WIP: Generate rayfiles\n");
          break;
        case SAVE:
          save_extension(t, op[i].op.save.p->name);

          break;
        case SHADING:
          printf("WIP: Shading\n");
          break;
        case SETKNOBS:
          printf("WIP: Set knobs\n");
          break;
        case FOCAL:
          printf("WIP: Focal\n");
          break;
        case DISPLAY:
          display(t);
          break;
        }
  
    //printf("\n");
  }
}
