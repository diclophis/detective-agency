#include <stdio.h>
#include <stdlib.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>
#include <mruby/compile.h>
#include <mruby/string.h>

#include "build/init.h"

static mrb_value business(mrb_state* mrb, mrb_value obj)
{
  mrb_value ret;
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  ret = mrb_yield_argv(mrb, block, 0, NULL);

  char *tasks = mrb_str_to_cstr(mrb, ret);
  fprintf(stdout, tasks);

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

  mrb_define_method(mrb, mrb->object_class, "business", business, MRB_ARGS_BLOCK());

  int bundled_ruby_count = 1;
  uint8_t const **bundled_ruby = (uint8_t const **)malloc(sizeof(const uint8_t *) * bundled_ruby_count);

  bundled_ruby[0] = init;

  for (i=0; i<bundled_ruby_count; i++) {
    // load the compiled library
    ret = mrb_load_irep(mrb, bundled_ruby[i]);

    // check for exception
    if (mrb->exc)
    {
      // print exception
      mrb_p(mrb, mrb_obj_value(mrb->exc));
      return 1;
    }
  }

  FILE *f = 0;
  f = fopen("Detectivefile", "r");
  if (0 != f) {
    ret = mrb_load_file(mrb, f);
    fclose(f);

    // check for exception
    if (mrb->exc)
    {
      // print exception
      mrb_p(mrb, mrb_obj_value(mrb->exc));
    }
  }

  // cleanup
  mrb_close(mrb);
  return 0;
}
