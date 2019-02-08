import unittest
from unittest.mock.

from sia.core.core import _, token, new


class TestCore(unittest.TestCase):
    def test_core_undescore_value(self):
        '''
        Test that the value of _ is really None
        '''

        self.assertIsNone(_)

    def test_core_token_decorator(self):
        '''
        Test that the decorator token inject the
        rx argument into the __doc__ of the function
        '''

        regx = r'[a-z]'

        def afunc():
            pass

        self.assertEqual(token(regx)(afunc).__doc__, regx)

    def test_core_new_function(self):
        '''
        Test that the function new returns
        correctly an new instance of a particular
        class
        '''

        class AClass:
            def __init__(self, x):
                self.x = x

        self.assertEqual(new(AClass, 12).x, 12)


if __name__ == '__main__':
    unittest.main()
