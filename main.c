#include <stdio.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>
#include <mruby/compile.h>
#include <mruby/string.h>

#include "build/init.h"
#include "build/wang.h"

static mrb_value biz(mrb_state* mrb, mrb_value obj)
{
  mrb_value ret;
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  fprintf(stderr, "Before block\n");

  ret = mrb_yield_argv(mrb, block, 0, NULL);

  mrb_p(mrb, ret);

  //fprintf(stderr, mrb_sprintf(mrb, "wtf: %s", ret));
  char *path;
  path = mrb_str_to_cstr(mrb, ret);
  fprintf(stderr, path);

  fprintf(stderr, "After block\n");

  return mrb_nil_value();
}


int main(int argc, char** argv) {
  mrb_state *mrb;
  mrb_value ret;

  // initialize mruby
  if (!(mrb = mrb_open()))
  {
    fprintf(stderr,"%s: could not initialize mruby\n",argv[0]);
    return -1;
  }

  mrb_value args = mrb_ary_new(mrb);
  int i;

  // convert argv into mruby strings
  for (i=1; i<argc; i++)
  {
     mrb_ary_push(mrb, args, mrb_str_new_cstr(mrb,argv[i]));
  }

  mrb_define_global_const(mrb, "ARGV", args);

  mrb_define_method(mrb, mrb->object_class, "biz", biz, MRB_ARGS_BLOCK());

  // load the compiled library
  ret = mrb_load_irep(mrb, wang);
  // check for exception
  if (mrb->exc)
  {
    // print exception
    mrb_p(mrb, mrb_obj_value(mrb->exc));
  }

  // load the compiled library
  ret = mrb_load_irep(mrb, init);
  // check for exception
  if (mrb->exc)
  {
    // print exception
    mrb_p(mrb, mrb_obj_value(mrb->exc));
  }

  FILE *f = fopen("chung", "r");
  ret = mrb_load_file(mrb, f);
  fclose(f);

  // check for exception
  if (mrb->exc)
  {
    // print exception
    mrb_p(mrb, mrb_obj_value(mrb->exc));
  }

  // cleanup
  mrb_close(mrb);
  return 0;
}
