#include <ruby.h>
#include <stdbool.h>

#include "atomic_boolean.h"

// http://gcc.gnu.org/onlinedocs/gcc-4.7.1/gcc/_005f_005fatomic-Builtins.html

VALUE atomic_boolean_allocate(VALUE klass) {
  CAtomicBoolean* atomic;
  VALUE sval = Data_Make_Struct(klass, CAtomicBoolean, NULL, atomic_boolean_deallocate, atomic);

#ifndef __USE_GCC_ATOMIC
  pthread_mutex_init(&atomic->mutex, NULL);
#endif

  return(sval);
}

void atomic_boolean_deallocate(void* sval) {
  CAtomicBoolean* atomic = (CAtomicBoolean*) sval;

#ifndef __USE_GCC_ATOMIC
  pthread_mutex_destroy(&atomic->mutex);
#endif

  free(atomic);
}

VALUE method_atomic_boolean_initialize(int argc, VALUE* argv, VALUE self) {
  CAtomicBoolean* atomic;

  rb_check_arity(argc, 0, 1);
  /*if (argc == 1) Check_Type(argv[0], T_BOOLEAN);*/

  Data_Get_Struct(self, CAtomicBoolean, atomic);

  atomic->value = (argc == 1 ? RUBY2BOOL(argv[0]) : Qfalse);
  return(self);
}

VALUE method_atomic_boolean_value(VALUE self) {
  CAtomicBoolean* atomic;
  bool value;

  Data_Get_Struct(self, CAtomicBoolean, atomic);

#ifdef __USE_GCC_ATOMIC
  value = __atomic_load_n(&atomic->value, __ATOMIC_SEQ_CST);
#else
  pthread_mutex_lock(&atomic->mutex);
  value = atomic->value;
  pthread_mutex_unlock(&atomic->mutex);
#endif

  return BOOL2RUBY(value);
}

VALUE method_atomic_boolean_value_eq(VALUE self, VALUE value) {
  CAtomicBoolean* atomic;
  bool new_value;

  /*Check_Type(value, T_BOOLEAN);*/

  new_value = RUBY2BOOL(value);
  Data_Get_Struct(self, CAtomicBoolean, atomic);

#ifdef __USE_GCC_ATOMIC
  __atomic_store_n(&atomic->value, new_value, __ATOMIC_SEQ_CST);
#else
  pthread_mutex_lock(&atomic->mutex);
  atomic->value = new_value;
  pthread_mutex_unlock(&atomic->mutex);
#endif

  return(value);
}

VALUE method_atomic_boolean_true_question(VALUE self) {
  return(method_atomic_boolean_value(self));
}

VALUE method_atomic_boolean_false_question(VALUE self) {
  VALUE current = method_atomic_boolean_value(self);
  return(BOOL2RUBY(current == Qfalse));
}

VALUE method_atomic_boolean_make_true(VALUE self) {
  //NOTE: need to find best gcc atomic method
  return(Qfalse);
}

VALUE method_atomic_boolean_make_false(VALUE self) {
  //NOTE: need to find best gcc atomic method
  return(Qfalse);
}
