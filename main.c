#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


#include <mruby.h>
#include <mruby/array.h>
#include <mruby/irep.h>
#include <mruby/compile.h>
#include <mruby/string.h>
#include <mruby/error.h>


#include "detective.h"
#include "investigation.h"


static void if_exception_error_and_exit(mrb_state* mrb, char *msg) {
  // check for exception, only one can exist at any point in time
  if (mrb->exc) {
    fprintf(stderr, "%s", msg);
    mrb_print_error(mrb);
    exit(2);
  }
}


static void eval_static_libs(mrb_state* mrb, ...) {
  va_list argp;
  va_start(argp, mrb);

  int end_of_static_libs = 0;
  uint8_t const *p;

  while(!end_of_static_libs) {
    p = va_arg(argp, uint8_t const*);
    if (NULL == p) {
      end_of_static_libs = 1;
    } else {
      mrb_load_irep(mrb, p);
      if_exception_error_and_exit(mrb, "Exception in bundled ruby\n");
    }
  }

  va_end(argp);
}


static mrb_value business(mrb_state* mrb, mrb_value obj)
{
  mrb_value ret;
  mrb_value block;
  mrb_get_args(mrb, "&", &block);

  ret = mrb_yield_argv(mrb, block, 0, NULL);

  if_exception_error_and_exit(mrb, "Exception in business yield\n");

  char *tasks = mrb_str_to_cstr(mrb, ret);
  fprintf(stdout, "%s", tasks);

  return mrb_nil_value();
}


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

  mrb_define_method(mrb, mrb->object_class, "business", business, MRB_ARGS_BLOCK());

  eval_static_libs(mrb, detective, investigation, NULL);

  mrbc_context *detective_file = mrbc_context_new(mrb);
  mrbc_filename(mrb, detective_file, "Detectivefile");
  ret = mrb_load_file_cxt(mrb, f, detective_file);
  mrbc_context_free(mrb, detective_file);
  fclose(f);
  if_exception_error_and_exit(mrb, "Exception in Detectivefile\n");

  // cleanup mruby
  mrb_close(mrb);

  // cleanup our opened Detectivefile
  fclose(f);

  return 0;
}
