import unittest
import os
import subprocess
import time


class MyTestCase(unittest.TestCase):
    fsh = os.getenv("FSH")
    bash = "/bin/bash"

    def test_simple_cmd(self):
        args = ["echo test test"]
        f_sh = subprocess.Popen(
            [self.fsh, "-c"] + args, stdout=subprocess.PIPE)
        bash = subprocess.Popen(
            [self.bash, "-c"] + args, stdout=subprocess.PIPE)
        self.assertEqual(f_sh.communicate()[0], bash.communicate()[0])

    def test_pipeline(self):
        args = ["echo test test | grep test | cat"]
        f_sh = subprocess.Popen(
            [self.fsh, "-c"] + args, stdout=subprocess.PIPE)
        bash = subprocess.Popen(
            [self.bash, "-c"] + args, stdout=subprocess.PIPE)
        self.assertEqual(f_sh.communicate()[0], bash.communicate()[0])

    def test_redirections(self):
        args = ["ls hehe 2>1"]
        f_sh = subprocess.Popen(
            [self.fsh, "-c"] + args, stdout=subprocess.PIPE)
        bash = subprocess.Popen(
            [self.bash, "-c"] + args, stdout=subprocess.PIPE)
        self.assertEqual(f_sh.communicate()[0], bash.communicate()[0])


if __name__ == '__main__':
    unittest.main()
