/**
 * Create a cURL instance for HTTP requests.
 * 
 * **Note: cURL module must be loaded by calling LoadLibrary("curl") before using!**
 * 
 * @class
 */
function Curl() { }
/**
 * Set HTTP proxy to use. The parameter should be a string holding the host name or dotted IP address.
 * To specify port number in this string, append :[port] to the end of the host name. The proxy 
 * string may be prefixed with [protocol]:// since any such prefix will be ignored. 
 * The proxy's port number may optionally be specified with the separate function SetProxyPort().
 * 
 * @param {string} proxy the proxy.
 */
Curl.prototype.SetProxy = function (proxy) { };
/**
 * Pass a long with this option to set the proxy port to connect to unless it is specified in the proxy string using SetProxy().
 * 
 * @param {number} port the port number.
 */
Curl.prototype.SetProxyPort = function (port) { };
/**
 * Pass a string which should be [username]:[password] to use for BASIC_AUTH the connection to the HTTP proxy.
 * 
 * @param {string} user_pw basic auth authentication string to the proxy.
 */
Curl.prototype.SetProxyUser = function (user_pw) { };
/**
 * Switch between SOCKS and HTTP proxies.
 * 
 * @param {boolean} isSocks true to use a SOCKS proxy, false for an HTTP proxy.
 */
Curl.prototype.SetSocksProxy = function (isSocks) { };
/**
 * Pass string, which should be [username]:[password] to use for the connection. Using a colon with no password will make libcurl use an empty password.
 * might perform several requests to possibly different hosts. libcurl will only send this user and password information to hosts using the initial host
 * name (unless SetUnrestrictedAuth(true) is set), so if libcurl follows locations to other hosts it will not send the user and password to those.
 * This is enforced to prevent accidental information leakage.
 * @param {string} user_pw the username/password combination to use for BASIC_AUTH.
 */
Curl.prototype.SetUserPw = function (user_pw) { };
/**
 * This string will be used to set the User-Agent: header in the http request sent to the remote server.
 * This can be used to fool servers or scripts. You can also set any custom header with AddHeader().
 * Default: "cURL-DOjS <version>"
 * 
 * @param {string} agent the agent string to use.
 */
Curl.prototype.SetUserAgent = function (agent) { };
/**
 * This string will be used to set the Referer: header in the http request sent to the remote server.
 * This can be used to fool servers or scripts. You can also set any custom header with AddHeader().
 * 
 * @param {string} referer referer string to use.
 */
Curl.prototype.SetReferer = function (referer) { };
/**
 * The set number will be the redirection limit. If that many redirections have been followed, the next redirect will cause an error (CURLE_TOO_MANY_REDIRECTS).
 * This option only makes sense if the SetFollowLocation() is used at the same time.
 * @param {number} maximum max redirects
 */
Curl.prototype.SetMaxRedirs = function (maximum) { };
/**
 * A boolean parameter to tell the library it can continue to send authentication (user+password) when following locations, even when hostname changed.
 * Note that this is meaningful only when setting SetFollowLocation(true).
 * @param {boolean} unrestricted true to no not restrict authentication, false to restrict.
 */
Curl.prototype.SetUnrestrictedAuth = function (unrestricted) { };
/**
 * A boolean parameter to tell the library to follow any Location: header that the server sends as part of a HTTP header.
 * 
 * @param {boolean} doFollow true to follow redirects, false to ignore them.
 */
Curl.prototype.SetFollowLocation = function (doFollow) { };
/**
 * Pass a pointer to a zero terminated string as parameter. It will be used to set a cookie in the http request. 
 * The format of the string should be NAME=CONTENTS, where NAME is the cookie name and CONTENTS is what the cookie should contain.
 * If you need to set mulitple cookies, you need to set them all using a single option and thus you need to concat them all 
 * in one single string. Set multiple cookies in one string like this: "name1=content1;name2=content2;" etc. 
 * Using this option multiple times will only make the latest string override the previously ones.
 * 
 * @param {string} cookies a cookie string.
 */
Curl.prototype.SetCookies = function (cookies) { };
/**
 * The string should be the file name of your private key.
 * 
 * @param {string} key name of the PEM key file.
 */
Curl.prototype.SetKey = function (key) { };
/**
 * It will be used as the password required to use the SetKey() private key.
 * 
 * @param {string} key_pw password for key file.
 */
Curl.prototype.SetKeyPassword = function (key_pw) { };
/**
 * The string should be the file name of your certificate.
 * 
 * @param {string} cert name of the PEM cert file.
 */
Curl.prototype.SetCertificate = function (cert) { };
/**
 * It will be used as the password required to use the SetCertificate() certificate.
 * 
 * @param {string} cer_pw password for key file.
 */
Curl.prototype.SetCertificatePassword = function (cer_pw) { };
/**
 * A string naming a file holding one or more certificates to verify the peer with.
 * Default: "cacert.pem"
 * 
 * @param {string} cafile PEM cainfo file.
 */
Curl.prototype.SetCaFile = function (cafile) { };
/**
 * It should contain the maximum time in seconds that you allow the connection to the server to take. 
 * This only limits the connection phase, once it has connected, this option is of no more use. 
 * Set to zero to disable connection timeout (it will then only timeout on the system's internal timeouts). 
 * See also the SetTimeout() option.
 * 
 * @param {number} timeout the timeout in seconds.
 */
Curl.prototype.SetConnectTimeout = function (timeout) { };
/**
 * The maximum time in seconds that you allow the libcurl transfer operation to take. 
 * Normally, name lookups can take a considerable time and limiting operations to less than a few minutes risk aborting perfectly normal operations.
 * 
 * @param {number} timeout the timeout in seconds.
 */
Curl.prototype.SetTimeout = function (timeout) { };
/**
 * Pass false to stop curl from verifying the peer's certificate.
 * 
 * @param {boolean} verify true to verify the peers certificates, false to ignore certificate errors.
 */
Curl.prototype.SetSslVerify = function (verify) { };
/**
 * Add a header in the form "Name:Value" to all requests done with this instance of Curl.
 * If you add a header that is otherwise generated and used by libcurl internally, your added one will be used instead. 
 * If you add a header with no contents as in 'Accept:' (no data on the right side of the colon), the internally used 
 * header will get disabled. Thus, using this option you can add new headers, replace internal headers and remove internal headers.
 * The headers must not be CRLF-terminated, because curl adds CRLF after each header item. Failure to comply with this will result in strange bugs because the server will most likely ignore part of the headers you specified.
 * 
 * @param {string} header a header to add.
 */
Curl.prototype.AddHeader = function (header) { };
/**
 * Clear all currently used headers previously set using AddHeader().
 */
Curl.prototype.ClearHeaders = function () { };
/**
 * Switch this Curl instance to HTTP_GET (the default).
 */
Curl.prototype.SetGet = function () { };
/**
 * Switch this Curl instance to HTTP_POST.
 * post_data should be the full data to post in a HTTP post operation. 
 * This data will be sent with every request until either HTTP_POST is disabled using SetGet() or new data ist set using this method.
 * You need to make sure that the data is formatted the way you want the server to receive it. 
 * libcurl will not convert or encode it for you. Most web servers will assume this data to be url-encoded (see urlencode()). Take note.
 * @see urlencode()
 *
 * @param {string} post_data the POST data to send.
 */
Curl.prototype.SetPost = function (post_data) { };
/**
 * Switch this Curl instance to HTTP_PUT. 
 * This data will be sent with every request until either HTTP_PUT is disabled using SetGet() or new data ist set using this method.
 * 
 * @param {ByteArray} put_data the PUT data to send.
 */
Curl.prototype.SetPut = function (put_data) { };
/**
 * @return {string} return the last effectively used URL.
 */
Curl.prototype.GetLastUrl = function () { };
/**
 * @return {number} get the last generated response code.
 */
Curl.prototype.GetResponseCode = function () { };
/**
 * perform a request with all settings previously set using the other methods.
 * 
 * @param {string} url the URL to connect to.
 * 
 * @returns {ByteArray[]} An array with two IntArrays and the response code. The first (index 0) contains the request body, the second (index 1) the request headers and the third (index 2) the response code.
 */
Curl.prototype.DoRequest = function (url) { };
/**
 * Add mime multipart POST data to a POST request.
 * 
 * @param {String} name name of the mime attachment.
 * @param {String|ByteArray} data the data to transmit.
 * @param {String} [mimetype] optional mime type of the data
 * @param {String} [filename] optional filename of the data
 */
Curl.prototype.AddPostData = function (name, data, mimetype, filename) { };
/**
 * Clear all currently used POST data previously set using AddPostData().
 */
Curl.prototype.ClearPostData = function () { };
