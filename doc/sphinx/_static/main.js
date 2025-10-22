/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                     *
 * Copyright (C) 2017 Georgi Yatsev <georgi.yatsev@guh.io>                             *
 * Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                                *
 *                                                                                     *
 * Permission is hereby granted, free of charge, to any person obtaining a copy        *
 * of this software and associated documentation files (the "Software"), to deal       *
 * in the Software without restriction, including without limitation the rights        *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell           *
 * copies of the Software, and to permit persons to whom the Software is               *
 * furnished to do so, subject to the following conditions:                            *
 *                                                                                     *
 * The above copyright notice and this permission notice shall be included in all      *
 * copies or substantial portions of the Software.                                     *
 *                                                                                     *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR          *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,            *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE         *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER              *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,       *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE       *
 * SOFTWARE.                                                                           *
 *                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

function onSwitchChanged() {
    setDarkTheme(document.getElementById("themeSwitch").checked);
}

function setDarkTheme(enabled) {
    if (enabled) {
      switchTheme("style/main-dark.css")
    } else {
      switchTheme("style/main.css")
    }

    localStorage.setItem("darkTheme", enabled)
}

function switchTheme(themeName) {
    console.log("Theme switched: " + themeName)
    document.getElementById("theme").setAttribute("href", themeName);
}

function onLoaded() {
    var darkThemeEnabled = JSON.parse(localStorage.getItem('darkTheme')) || false

    console.log("Initialize theme: " + (darkThemeEnabled ? "Dark theme" : "Light theme"))
    setDarkTheme(darkThemeEnabled)

    console.log("Set theme switch " + darkThemeEnabled)
    var themeSwitch = document.getElementById("themeSwitch");
    if (darkThemeEnabled) {
        themeSwitch.checked = true;
    } else {
        themeSwitch.checked = false;
    }
}

document.onreadystatechange = function () {
    console.log("--> " + document.readyState)
}

document.addEventListener('DOMContentLoaded', function() {
    onLoaded()
});

(function (root, factory) {
    if (typeof define === "function" && define.amd) {
        define([], factory())
    } else if (typeof module === "object" && module.exports) {
        module.exports = factory()
    } else {
        (function install() {
            if (document && document.body) {
                root.zenscroll = factory()
            } else {
                setTimeout(install, 9)
            }
        })()
    }
}(this, function () {
    "use strict"


    var isNativeSmoothScrollEnabledOn = function (elem) {
        return ("getComputedStyle" in window) &&
            window.getComputedStyle(elem)["scroll-behavior"] === "smooth"
    }


    if (typeof window === "undefined" || !("document" in window)) {
        return {}
    }


    var makeScroller = function (container, defaultDuration, edgeOffset) {

        defaultDuration = defaultDuration || 999 //ms
        if (!edgeOffset && edgeOffset !== 0) {
            edgeOffset = 9 //px
        }

        var scrollTimeoutId
        var setScrollTimeoutId = function (newValue) {
            scrollTimeoutId = newValue
        }


        var stopScroll = function () {
            clearTimeout(scrollTimeoutId)
            setScrollTimeoutId(0)
        }

        var getTopWithEdgeOffset = function (elem) {
            return Math.max(0, container.getTopOf(elem) - edgeOffset)
        }

        var scrollToY = function (targetY, duration, onDone) {
            stopScroll()
            if (duration === 0 || (duration && duration < 0) || isNativeSmoothScrollEnabledOn(container.body)) {
                container.toY(targetY)
                if (onDone) {
                    onDone()
                }
            } else {
                var startY = container.getY()
                var distance = Math.max(0, targetY) - startY - 100
                var startTime = new Date().getTime()
                duration = duration || Math.min(Math.abs(distance), defaultDuration);
                (function loopScroll() {
                    setScrollTimeoutId(setTimeout(function () {
                        // Calculate percentage:
                        var p = Math.min(1, (new Date().getTime() - startTime) / duration)
                        // Calculate the absolute vertical position:
                        var y = Math.max(0, Math.floor(startY + distance*(p < 0.5 ? 2*p*p : p*(4 - p*2)-1)))
                        container.toY(y)
                        if (p < 1 && (container.getHeight() + y) < container.body.scrollHeight) {
                            loopScroll()
                        } else {
                            setTimeout(stopScroll, 99) // with cooldown time
                            if (onDone) {
                                onDone()
                            }
                        }
                    }, 9))
                })()
            }
        }

        var scrollToElem = function (elem, duration, onDone) {
            scrollToY(getTopWithEdgeOffset(elem), duration, onDone)
        }

        var scrollIntoView = function (elem, duration, onDone) {
            var elemHeight = elem.getBoundingClientRect().height
            var elemBottom = container.getTopOf(elem) + elemHeight
            var containerHeight = container.getHeight()
            var y = container.getY()
            var containerBottom = y + containerHeight
            if (getTopWithEdgeOffset(elem) < y || (elemHeight + edgeOffset) > containerHeight) {
                // Element is clipped at top or is higher than screen.
                scrollToElem(elem, duration, onDone)
            } else if ((elemBottom + edgeOffset) > containerBottom) {
                // Element is clipped at the bottom.
                scrollToY(elemBottom - containerHeight + edgeOffset, duration, onDone)
            } else if (onDone) {
                onDone()
            }
        }

        var scrollToCenterOf = function (elem, duration, offset, onDone) {
            scrollToY(Math.max(0, container.getTopOf(elem) - container.getHeight()/2 + (offset || elem.getBoundingClientRect().height/2)), duration, onDone)
        }

        var setup = function (newDefaultDuration, newEdgeOffset) {
            if (newDefaultDuration === 0 || newDefaultDuration) {
                defaultDuration = newDefaultDuration
            }
            if (newEdgeOffset === 0 || newEdgeOffset) {
                edgeOffset = newEdgeOffset
            }
            return {
                defaultDuration: defaultDuration,
                edgeOffset: edgeOffset
            }
        }

        return {
            setup: setup,
            to: scrollToElem,
            toY: scrollToY,
            intoView: scrollIntoView,
            center: scrollToCenterOf,
            stop: stopScroll,
            moving: function () { return !!scrollTimeoutId },
            getY: container.getY,
            getTopOf: container.getTopOf
        }

    }


    var docElem = document.documentElement
    var getDocY = function () { return window.scrollY || docElem.scrollTop }

    var zenscroll = makeScroller({
        body: document.scrollingElement || document.body,
        toY: function (y) { window.scrollTo(0, y) },
        getY: getDocY,
        getHeight: function () { return window.innerHeight || docElem.clientHeight },
        getTopOf: function (elem) { return elem.getBoundingClientRect().top + getDocY() - docElem.offsetTop }
    })

    zenscroll.createScroller = function (scrollContainer, defaultDuration, edgeOffset) {
        return makeScroller({
            body: scrollContainer,
            toY: function (y) { scrollContainer.scrollTop = y },
            getY: function () { return scrollContainer.scrollTop },
            getHeight: function () { return Math.min(scrollContainer.clientHeight, window.innerHeight || docElem.clientHeight) },
            getTopOf: function (elem) { return elem.offsetTop }
        }, defaultDuration, edgeOffset)
    }

    if ("addEventListener" in window && !window.noZensmooth && !isNativeSmoothScrollEnabledOn(document.body)) {


        var isScrollRestorationSupported = "scrollRestoration" in history

        if (isScrollRestorationSupported) {
            history.scrollRestoration = "auto"
        }

        window.addEventListener("load", function () {

            if (isScrollRestorationSupported) {
                setTimeout(function () { history.scrollRestoration = "manual" }, 9)
                window.addEventListener("popstate", function (event) {
                    if (event.state && "zenscrollY" in event.state) {
                        zenscroll.toY(event.state.zenscrollY)
                    }
                }, false)
            }

            if (window.location.hash) {
                setTimeout(function () {
                    var edgeOffset = zenscroll.setup().edgeOffset
                    if (edgeOffset) {
                        var targetElem = document.getElementById(window.location.href.split("#")[1])
                        if (targetElem) {
                            var targetY = Math.max(0, zenscroll.getTopOf(targetElem) - edgeOffset)
                            var diff = zenscroll.getY() - targetY
                            if (0 <= diff && diff < 9 ) {
                                window.scrollTo(0, targetY)
                            }
                        }
                    }
                }, 9)
            }

        }, false)

        var RE_noZensmooth = new RegExp("(^|\\s)noZensmooth(\\s|$)")
        window.addEventListener("click", function (event) {
            var anchor = event.target
            while (anchor && anchor.tagName !== "A") {
                anchor = anchor.parentNode
            }
            if (!anchor || event.which !== 1 || event.shiftKey || event.metaKey || event.ctrlKey || event.altKey) {
                return
            }
            if (isScrollRestorationSupported) {
                try {
                    history.replaceState({ zenscrollY: zenscroll.getY() }, "")
                } catch (e) {
                }
            }
            var href = anchor.getAttribute("href") || ""
            if (href.indexOf("#") === 0 && !RE_noZensmooth.test(anchor.className)) {
                var targetY = 0
                var targetElem = document.getElementById(href.substring(1))
                if (href !== "#") {
                    if (!targetElem) {
                        return
                    }
                    targetY = zenscroll.getTopOf(targetElem)
                }
                event.preventDefault()
                var onDone = function () { window.location = href }
                var edgeOffset = zenscroll.setup().edgeOffset
                if (edgeOffset) {
                    targetY = Math.max(0, targetY - edgeOffset)
                    onDone = function () { history.pushState(null, "", href) }
                }
                zenscroll.toY(targetY, null, onDone)
            }
        }, false)

    }


    return zenscroll


}));
