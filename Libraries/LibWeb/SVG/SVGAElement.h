/*
 * Copyright (c) 2024, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2024, Jamie Mansfield <jmansfield@cadixdev.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWeb/HTML/HTMLHyperlinkElementUtils.h>
#include <LibWeb/SVG/SVGGraphicsElement.h>
#include <LibWeb/SVG/SVGURIReference.h>

namespace Web::SVG {

class HTMLHyperlinkElementUtilsHack : public Web::HTML::HTMLHyperlinkElementUtils {
public:
    virtual ~HTMLHyperlinkElementUtilsHack();
    HTMLHyperlinkElementUtilsHack(DOM::Document& document, DOM::Element& element)
        : m_document(document)
        , m_element(element)
    {
    }

private:
    DOM::Document& m_document;
    DOM::Element& m_element;

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
        (void)target;
        // return m_html_element.get_an_elements_target(target);
        return "_self"_string;
    }
    virtual Web::HTML::TokenizedFeature::NoOpener hyperlink_element_utils_get_an_elements_noopener(URL::URL const& url, StringView target) const override
    {
        (void)url;
        (void)target;
        // return m_html_element.get_an_elements_noopener(url, target);
        return Web::HTML::TokenizedFeature::NoOpener::Yes;
    }
};

class SVGAElement final
    : public SVGGraphicsElement
    , public SVGURIReferenceMixin<SupportsXLinkHref::Yes> {
    WEB_PLATFORM_OBJECT(SVGAElement, SVGGraphicsElement);
    GC_DECLARE_ALLOCATOR(SVGAElement);

public:
    virtual ~SVGAElement() override;

    GC::Ref<SVGAnimatedString> target();

    GC::Ref<DOM::DOMTokenList> rel_list();

    virtual GC::Ptr<Layout::Node> create_layout_node(GC::Ref<CSS::ComputedProperties>) override;

private:
    SVGAElement(DOM::Document&, DOM::QualifiedName);

    virtual void initialize(JS::Realm&) override;
    virtual void visit_edges(Cell::Visitor&) override;

    virtual bool is_svg_a_element() const override { return true; }

    bool has_download_preference() const;

    // ^DOM::EventTarget
    virtual bool has_activation_behavior() const override;
    virtual void activation_behavior(Web::DOM::Event const&) override;

    // ^DOM::Element
    virtual void attribute_changed(FlyString const& name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_) override;
    virtual i32 default_tab_index_value() const override;

    GC::Ptr<DOM::DOMTokenList> m_rel_list;

    GC::Ptr<SVGAnimatedString> m_target;

    HTMLHyperlinkElementUtilsHack* m_hyperlink_utils;
};

}

namespace Web::DOM {

template<>
inline bool Node::fast_is<SVG::SVGAElement>() const { return is_svg_a_element(); }

}
