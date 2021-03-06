#ifndef FIELD_H
#define FIELD_H

#include <iostream>
#include <stdexcept>

struct Field {
 public:
  enum Letter {A, B, C, D, E, F, G, H, NONE};
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

  Field() : letter(A), number(ONE) {}

  Field(Letter l, Number n) : letter(l), number(n) {
    if (l < A || l > H || n < ONE || n > EIGHT) {
      throw WrongFieldException(l, n);
    }
  }

  explicit Field(const char* f) : Field(static_cast<Letter>(f[0] - 'a'),
                                        static_cast<Number>(f[1] - '1')) {
  }

  static bool isFieldValid(const std::string& f) {
    return f.size() == 2 && f[0] >= 'a' && f[0] <= 'h' && f[1] >= '1' && f[1] <= '8';
  }

  Letter letter;
  Number number;
};

inline std::ostream& operator<<(std::ostream& ostr, const Field& field) {
  char letter = field.letter + 'a';
  char number = field.number + '1';
  if (letter < 'a' || letter > 'h' || number < '1' || number > '8') {
    throw Field::WrongFieldException(field.letter, field.number);
  }
  ostr << letter << number;
  return ostr;
}

#endif  // FIELD_H
