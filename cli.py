import kvstore as kv
import cmd
import client
import sys

def parse(arg):
    return tuple(map(str, arg.split()))

class CLI(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.prompt = '> '
        # self.client = client.Client(('localhost', 10000))

    def do_connect(self, s):
        self.client = client.Client(('localhost', int(s)))

    def do_EOF(self, s):
        sys.exit(0)

    def do_del(self, key):
        kv.do_del(key)

    def help_del(self):
        print "syntax: del [key]",
        print "-- deletes the entry for [key] from the database if it exists"

    def do_get(self, key):
        self.client.get(key)
        # kv.do_get(key)

    def help_get(self):
        print "syntax: get [key]",
        print "-- gets the value associated with [key] from the database if it exists, nil if it doesnt."

    def do_set(self, arg):
        kv.do_set(*parse(arg))

    def help_set(self):
        print "syntax: set [key] [val]",
        print "-- maps [key] to [val] in the database. Will overwrite existing value if [key] is already mapped to something."

    def do_getset(self, arg):
        kv.do_getset(*parse(arg))

    def help_getset(self):
        print "syntax: getset [key] [val]",
        print "-- will get the value associated with [key] and then replace it with [val]"

    def do_quit(self, arg):
        sys.exit(0)

    def help_quit(self):
        print "syntax: quit",
        print "-- terminates the application"

if __name__=="__main__":
    cli = CLI()
    cli.cmdloop()
