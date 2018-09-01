#ifndef UTIL_H
#define UTIL_H

#define MIN(X, Y) \
    __extension__ ({ __typeof__(X) _X = (X); __typeof__(Y) _Y = (Y); _X < _Y ? _X : _Y; })

#endif /* UTIL_H */
