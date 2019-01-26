# web-traffic-filtering

# Requirements
- Linux
- Firefox at least version 57.0a1
- g++ with support for c++14
- gnu make

My configuration is:
- Ubuntu 16.04.4 LTS xenial
- Firefox 64.0
- g++ (Ubuntu 5.4.0-6ubuntu1~16.04.10) 5.4.0 20160609
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

Run `git submodule init` and `git submodule update` to get the dependencies.

# How to build
Run `make` in the project's root folder.

# How to make it work
- install extension in firefox - https://developer.mozilla.org/en-US/docs/Mozilla/Add-ons/WebExtensions/Temporary_Installation_in_Firefox (select `manifest.json` from `<root_folder>/extension` folder)
- move `<root_folder>/native-receiver/native_receiver.json` to `/usr/lib/mozilla/native-messaging-hosts/native_receiver.json`but not before editing the`path` field from `native_receiver.json`; change `<your-path>` with the full path to the project's root folder

# How to test it
- by default it filters any request to `http://www.phiorg.ro/` (coming soon and the https version)
- if you want to filter more sites or all sites you can edit the `filteredUrls` variable from the `<root_folder>/extension/background.js` file; use `<all_urls>` string to enable the extension for all sites; or just add another site in that array.
- now, just access any url that is filtered and you should see the `modified content` message instead of the content sent by the server.
