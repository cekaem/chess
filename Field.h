#ifndef FIELD_H
#define FIELD_H

#include <iostream>
#include <stdexcept>

struct Field {
 public:
  enum Letter {A, B, C, D, E, F, G, H};
  enum Number { ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT};

  struct WrongFieldException : std::exception {
    WrongFieldException(Letter l, Number n) : letter(l), number(n) {}
    const Letter letter;
    const Number number;
  };

  bool operator==(const Field& other) const {
    return letter == other.letter && number == other.number;
  }

  bool operator!=(const Field& other) const {
    return !(*this == other);
  }

  Field(Letter l, Number n) : letter(l), number(n) {
    if (l < A || l > H || n < ONE || n > EIGHT) {
      throw WrongFieldException(l, n);
    }
  }

  Letter letter;
  Number number;
};

inline std::ostream& operator<<(std::ostream& ostr, const Field& field) {
  ostr << "(" << field.letter << ", " << field.number << ")";
  return ostr;
}

#endif  // FIELD_H
