# web-traffic-filtering

# Requirements
- Linux
- Firefox at least version 57.0a1
- g++-7
- gnu make

My configuration is:
- Ubuntu 16.04.4 LTS xenial
- Firefox 64.0
- g++-7 (Ubuntu 7.4.0-1ubuntu1~16.04~ppa1) 7.4.0
- GNU Make 4.1

# Dependencies
- boost.asio (https://github.com/boostorg/asio)
- boost.assert (https://github.com/boostorg/assert)
- boost.bind (https://github.com/boostorg/bind)
- boost.config (https://github.com/boostorg/config)
- boost.core (https://github.com/boostorg/core)
- boost.date_time (https://github.com/boostorg/date_time)
- boost.mpl (https://github.com/boostorg/mpl)
- boost.numeric_conversion (https://github.com/boostorg/numeric_conversion)
- boost.predef (https://github.com/boostorg/predef)
- boost.preprocessor (https://github.com/boostorg/preprocessor)
- boost.regex (https://github.com/boostorg/regex)
- boost.smart_ptr (https://github.com/boostorg/smart_ptr)
- boost.static_assert (https://github.com/boostorg/static_assert)
- boost.system (https://github.com/boostorg/system)
- boost.throw_exception (https://github.com/boostorg/throw_exception)
- boost.type_traits (https://github.com/boostorg/type_traits)
- boost.utility (https://github.com/boostorg/utility)
- nlohmann.json (https://github.com/nlohmann/json)

Run `git submodule init` and `git submodule update` to get the dependencies.

# How to build
Run `make` in the project's root folder.

# How to run
- install extension in firefox - https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Temporary_Installation_in_Firefox (select `manifest.json` from `<root_folder>/extension` folder)
- move `<root_folder>/native-receiver/native_receiver.json` to `/usr/lib/mozilla/native-messaging-hosts/native_receiver.json`but not before editing the`path` field from `native_receiver.json`; change `<your-path>` with the full path to the project's root folder
- by default it filters any request to `http://www.phiorg.ro/` (coming soon and the https version)
- if you want to filter more sites or all sites you can edit the `filteredUrls` variable from the `<root_folder>/extension/background.js` file; use `<all_urls>` string to enable the extension for all sites; or just add another site in that array.
- url for test:
  - http://www.phiorg.ro/web_traffic_filtering/cancel_request - the request will be cancelled
  - http://www.phiorg.ro/web_traffic_filtering/redirect_request - the request will be redirected to http://www.phiorg.ro/web_traffic_filtering/redirected_request
  - http://www.phiorg.ro/web_traffic_filtering/modify_client_headers - the request will have a new header named "scan-engine" with the value "demo"
  - http://www.phiorg.ro/web_traffic_filtering/modify_server_headers - the response will have a cookie "scan-engine=web-traffic-filtering-demo"
  - http://www.phiorg.ro/web_traffic_filtering/modify_server_response - the response's body will be modified
  - http://www.phiorg.ro/web_traffic_filtering/change_server_response - the response's body will be changed in "blocked by web-traffic-filtering-demo"
