import unittest
from quad_to_py import translate_quads_to_python

class TestQuadToPythonTranslation(unittest.TestCase):

    def strip_comments_and_blank(self, lines):
        return [line.strip() for line in lines if line.strip() and not line.strip().startswith("#")]

    def test_simple_arithmetic(self):
        quads = [
            "0: ASSIGN x = 5",
            "1: ASSIGN y = 10",
            "2: ADD z = x + y",
            "3: PRINT \"Resultado: %d\", z"
        ]

        expected = [
            "x = 5",
            "y = 10",
            "z = x + y",
            "print(\"Resultado: %d\" % z)"
        ]

        output = self.strip_comments_and_blank(translate_quads_to_python(quads))
        self.assertEqual(output, expected)

    def test_sub_mul_div(self):
        quads = [
            "0: ASSIGN a = 8",
            "1: ASSIGN b = 2",
            "2: SUB c = a - b",
            "3: MUL d = a * b",
            "4: DIV e = a / b",
            "5: PRINT \"C: %d\", c",
            "6: PRINT \"D: %d\", d",
            "7: PRINT \"E: %d\", e"
        ]

        expected = [
            "a = 8",
            "b = 2",
            "c = a - b",
            "d = a * b",
            "e = a // b # Using integer division",
            "print(\"C: %d\" % c)",
            "print(\"D: %d\" % d)",
            "print(\"E: %d\" % e)"
        ]

        output = self.strip_comments_and_blank(translate_quads_to_python(quads))
        self.assertEqual(output, expected)

    def test_uminus_and_mod(self):
        quads = [
            "0: ASSIGN x = 10",
            "1: UMINUS y = - x",
            "2: MOD z = x % y",
            "3: PRINT \"Z: %d\", z"
        ]

        expected = [
            "x = 10",
            "y = -x",
            "z = x % y",
            "print(\"Z: %d\" % z)"
        ]

        output = self.strip_comments_and_blank(translate_quads_to_python(quads))
        self.assertEqual(output, expected)

if __name__ == '__main__':
    unittest.main()
