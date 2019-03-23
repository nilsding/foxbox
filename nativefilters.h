#ifndef NATIVEFILTERS_H
#define NATIVEFILTERS_H

#define HAVE_NATIVE_FILTERS

#ifdef Q_OS_MAC
  #include "nativefilters_mac.h"
#else
  #undef HAVE_NATIVE_FILTERS
#endif

#endif // NATIVEFILTERS_H
