#include <stdio.h>
#include <stdlib.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/error.h>

int main(int argc, char** argv) {
  FILE *f = 0;
  f = fopen("Detectivefile", "r");
  if (0 == f) {
    return 1;
  }

  mrb_state *mrb;
  mrb_value ret;

  // initialize mruby
  if (!(mrb = mrb_open())) {
    fprintf(stderr,"%s: could not initialize mruby\n",argv[0]);
    return -1;
  }

  mrb_value args = mrb_ary_new(mrb);
  int i;

  // convert argv into mruby strings
  for (i=1; i<argc; i++) {
     mrb_ary_push(mrb, args, mrb_str_new_cstr(mrb,argv[i]));
  }

  mrb_define_global_const(mrb, "ARGV", args);

  // cleanup mruby
  mrb_close(mrb);

  // cleanup our opened Detectivefile
  fclose(f);

  return 0;
}
