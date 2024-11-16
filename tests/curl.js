/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

LoadLibrary("curl");

function sslTest(url) {
	Println("+++ Trying " + url)

	var https = new Curl();
	var resp = https.DoRequest(url);
	assert("HTTPS test", resp[2], 200);
	Println(resp[1].ToString());

	Println("+++ DONE " + url)
}

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(1);
	Println(">>> date=" + new Date().toISOString());

	// simple https get
	sslTest("https://curl.se");
	sslTest("https://mastodon.social/about");
	sslTest("https://retrochat.online/about");
	sslTest("https://bitbang.social/about");
	sslTest("https://raw.githubusercontent.com/SuperIlu/DOjSHPackages/master/dojs/index.json");
	sslTest("https://www.shdon.com/");
	sslTest("https://www.heise.de");

	// try https post
	var post = new Curl();
	post.SetPost();
	post.AddPostData('FOO', 'BAR');
	var resp = post.DoRequest("https://curl.se");
	assert("HTTPS test", resp[2], 200);
	Println(resp[1].ToString());

	// try simple HTTP-GET
	var get = new Curl();
	var resp = get.DoRequest("http://192.168.2.8:8081/basic_test");
	assert("basic test", resp[2], 200);
	assert("basic test", resp[0].ToString(), "Basic OK");

	// fail 404
	var resp = get.DoRequest("http://192.168.2.8:8081/NOTFOUND");
	assert("fail 404", resp[2], 404);

	// add a header-field
	get.AddHeader("Foo: Bar");
	var resp = get.DoRequest("http://192.168.2.8:8081/header_test");
	assert("header test", resp[2], 200);
	assert("header test", resp[0].ToString(), "Header OK: Bar");

	// try get with parameters
	var get = new Curl();
	var resp = get.DoRequest("http://192.168.2.8:8081/get_test?foo=bar");
	assert("get test", resp[2], 200);
	assert("get test", resp[0].ToString(), "Get OK: {'foo': 'bar'}");

	// try simple post
	var POST_TST_STR = "foo=976765crcvrf(&*^&^78tyug65%$ytfTYr(TGUih";
	var post = new Curl();
	post.SetPost();
	var resp = post.DoRequest("http://192.168.2.8:8081/post_test");
	assert("post test", resp[2], 200);
	assert("post test", resp[0].ToString(), "Post OK: {'foo': '" + POST_TST_STR + "'}");

	Stop();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
}

function assert(txt, ist, soll) {
	if (ist != soll) {
		throw txt + ": result was '" + ist + "' but should be '" + soll + "'";
	}
}

/*
** This function is called on any input.
*/
function Input(event) {
}
