/*
 * Copyright (c) 2021, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2025, Logan Hunt
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibURL/URL.h>
#include <LibWeb/HTML/EventLoop/Task.h>
#include <LibWeb/HTML/HTMLHyperlinkElementUtils.h>
#include <LibWeb/HTML/TokenizedFeatures.h>
#include <LibWeb/HTML/UserNavigationInvolvement.h>
#include <LibWeb/SVG/SVGElement.h>

namespace Web::SVG {

class SVGHyperlinkElementUtils : public Web::HTML::HTMLHyperlinkElementUtils {
public:
    virtual ~SVGHyperlinkElementUtils();
    SVGHyperlinkElementUtils(SVGElement& element, DOM::Document& document);

private:
    SVGElement& m_element;
    DOM::Document& m_document;

    // ^HTML::HTMLHyperlinkElementUtils
    virtual DOM::Document& hyperlink_element_utils_document() override { return m_document; }
    virtual DOM::Element& hyperlink_element_utils_element() override { return m_element; }
    virtual Optional<String> hyperlink_element_utils_href() const override;
    virtual void set_hyperlink_element_utils_href(String) override;
    virtual Optional<String> hyperlink_element_utils_referrerpolicy() const override;
    virtual bool hyperlink_element_utils_is_html_anchor_element() const final { return true; }
    virtual bool hyperlink_element_utils_is_connected() const final { return m_element.is_connected(); }
    virtual void hyperlink_element_utils_queue_an_element_task(HTML::Task::Source source, Function<void()> steps) override
    {
        m_element.queue_an_element_task(source, move(steps));
    }
    virtual String hyperlink_element_utils_get_an_elements_target(Optional<String> target) const override
    {
        return m_element.get_an_elements_target(target);
    }
    virtual Web::HTML::TokenizedFeature::NoOpener hyperlink_element_utils_get_an_elements_noopener(URL::URL const& url, StringView target) const override
    {
        return m_element.get_an_elements_noopener(url, target);
    }
};

}
