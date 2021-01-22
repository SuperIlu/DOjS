from bottle import Bottle, run, request, auth_basic

app = Bottle()


def is_authenticated_user(user, password):
    return user == password


def dump_info(request):
    print(
        "\nURL={}, method={}\nheaders:\n  {}\nparams:\n  {}\nforms:\n  {}".format(
            request.url, request.method, dict(request.headers), dict(request.params), dict(request.forms)
        )
    )


@app.route("/basic_test")
def basic_test():
    return "Basic OK"


@app.route("/header_test")
def header_test():
    return "Header OK: {}".format(request.headers.get("Foo"))


@app.route("/auth_test")
@auth_basic(is_authenticated_user)
def auth_test():
    dump_info(request)
    return "Auth OK: {}".format(request.auth)


@app.route("/get_test", method="GET")
def get_test():
    dump_info(request)
    return "Get OK: {}".format(dict(request.query))


@app.route("/post_test", method="POST")
def post_test():
    dump_info(request)
    return "Post OK: {}".format(dict(request.forms))


@app.route("/put_test", method="PUT")
def put_test():
    return "Put OK: {}".format(request.body)


run(app, host="0.0.0.0", port=8080, debug=True, reloader=True)

