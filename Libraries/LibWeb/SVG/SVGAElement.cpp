/*
 * Copyright (c) 2024, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2024, Jamie Mansfield <jmansfield@cadixdev.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibURL/Parser.h>
#include <LibWeb/Bindings/SVGAElementPrototype.h>
#include <LibWeb/DOM/DOMTokenList.h>
#include <LibWeb/HTML/HTMLBaseElement.h>
#include <LibWeb/HTML/Navigable.h>
#include <LibWeb/HTML/TokenizedFeatures.h>
#include <LibWeb/HTML/UserNavigationInvolvement.h>
#include <LibWeb/Infra/CharacterTypes.h>
#include <LibWeb/Layout/SVGGraphicsBox.h>
#include <LibWeb/SVG/SVGAElement.h>
#include <LibWeb/UIEvents/MouseEvent.h>

namespace Web::HTML {

class Navigable;

}

namespace Web::SVG {

HTMLHyperlinkElementUtilsHack::~HTMLHyperlinkElementUtilsHack() = default;

Optional<String> HTMLHyperlinkElementUtilsHack::hyperlink_element_utils_href() const
{
    return m_element.attribute(HTML::AttributeNames::href);
}

void HTMLHyperlinkElementUtilsHack::set_hyperlink_element_utils_href(String href)
{
    m_element.set_attribute_value(HTML::AttributeNames::href, move(href));
}

Optional<String> HTMLHyperlinkElementUtilsHack::hyperlink_element_utils_referrerpolicy() const
{
    return m_element.attribute(HTML::AttributeNames::referrerpolicy);
}

GC_DEFINE_ALLOCATOR(SVGAElement);

SVGAElement::SVGAElement(DOM::Document& document, DOM::QualifiedName qualified_name)
    : SVGGraphicsElement(document, move(qualified_name))
{
}

SVGAElement::~SVGAElement() = default;

void SVGAElement::initialize(JS::Realm& realm)
{
    WEB_SET_PROTOTYPE_FOR_INTERFACE(SVGAElement);
    Base::initialize(realm);
}

void SVGAElement::visit_edges(Cell::Visitor& visitor)
{
    Base::visit_edges(visitor);
    SVGURIReferenceMixin::visit_edges(visitor);
    visitor.visit(m_rel_list);
    visitor.visit(m_target);
}

bool SVGAElement::has_activation_behavior() const
{
    return true;
}

// https://html.spec.whatwg.org/multipage/links.html#links-created-by-a-and-area-elements
void SVGAElement::activation_behavior(Web::DOM::Event const& event)
{
    // The activation behavior of an a or area element element given an event event is:

    // 1. If element has no href attribute, then return.
    if (href()->base_val().is_empty())
        return;

    // AD-HOC: Do not activate the element for clicks with the ctrl/cmd modifier present. This lets
    //         the browser process open the link in a new tab.
    if (is<UIEvents::MouseEvent>(event)) {
        auto const& mouse_event = static_cast<UIEvents::MouseEvent const&>(event);
        if (mouse_event.platform_ctrl_key())
            return;
    }

    // 2. Let hyperlinkSuffix be null.
    Optional<String> hyperlink_suffix {};

    // 3. If element is an a element, and event's target is an img with an ismap attribute specified, then:
    if (event.target() && is<SVGImageElement>(*event.target()) && static_cast<SVGImageElement const&>(*event.target()).has_attribute(Web::HTML::AttributeNames::ismap)) {
        // 1. Let x and y be 0.
        CSSPixels x { 0 };
        CSSPixels y { 0 };

        // 2. If event's isTrusted attribute is initialized to true, then set x to the distance in CSS pixels from the left edge of the image
        //    to the location of the click, and set y to the distance in CSS pixels from the top edge of the image to the location of the click.
        if (event.is_trusted() && is<UIEvents::MouseEvent>(event)) {
            auto const& mouse_event = static_cast<UIEvents::MouseEvent const&>(event);
            x = CSSPixels { mouse_event.offset_x() };
            y = CSSPixels { mouse_event.offset_y() };
        }

        // 3. If x is negative, set x to 0.
        x = max(x, 0);

        // 4. If y is negative, set y to 0.
        y = max(y, 0);

        // 5. Set hyperlinkSuffix to the concatenation of U+003F (?), the value of x expressed as a base-ten integer using ASCII digits,
        //    U+002C (,), and the value of y expressed as a base-ten integer using ASCII digits.
        hyperlink_suffix = MUST(String::formatted("?{},{}", x.to_int(), y.to_int()));
    }

    // 4. Let userInvolvement be event's user navigation involvement.
    auto user_involvement = Web::HTML::user_navigation_involvement(event);

    // 5. If the user has expressed a preference to download the hyperlink, then set userInvolvement to "browser UI".
    // NOTE: That is, if the user has expressed a specific preference for downloading, this no longer counts as merely "activation".
    if (has_download_preference())
        user_involvement = Web::HTML::UserNavigationInvolvement::BrowserUI;

    // FIXME: 6. If element has a download attribute, or if the user has expressed a preference to download the
    //     hyperlink, then download the hyperlink created by element with hyperlinkSuffix set to hyperlinkSuffix and
    //     userInvolvement set to userInvolvement.

    // DONE TO HERE

    // 7. Otherwise, follow the hyperlink created by element with hyperlinkSuffix set to hyperlinkSuffix and userInvolvement set to userInvolvement.

    if (m_hyperlink_utils == nullptr)
        m_hyperlink_utils = new HTMLHyperlinkElementUtilsHack(*this, document(), *this);

    m_hyperlink_utils->follow_the_hyperlink(hyperlink_suffix, user_involvement);
}

bool SVGAElement::has_download_preference() const
{
    return has_attribute(HTML::AttributeNames::download);
}

void SVGAElement::attribute_changed(FlyString const& name, Optional<String> const& old_value, Optional<String> const& value, Optional<FlyString> const& namespace_)
{
    Base::attribute_changed(name, old_value, value, namespace_);

    if (name == SVG::AttributeNames::href) {
        invalidate_style(
            DOM::StyleInvalidationReason::HTMLHyperlinkElementHrefChange,
            {
                { .type = CSS::InvalidationSet::Property::Type::PseudoClass, .value = CSS::PseudoClass::AnyLink },
                { .type = CSS::InvalidationSet::Property::Type::PseudoClass, .value = CSS::PseudoClass::Link },
                { .type = CSS::InvalidationSet::Property::Type::PseudoClass, .value = CSS::PseudoClass::LocalLink },
            },
            {});
    }
    if (name == HTML::AttributeNames::rel) {
        if (m_rel_list)
            m_rel_list->associated_attribute_changed(value.value_or(String {}));
    }
}

// https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
i32 SVGAElement::default_tab_index_value() const
{
    // See the base function for the spec comments.
    return 0;
}

// https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__target
GC::Ref<SVGAnimatedString> SVGAElement::target()
{
    if (!m_target)
        m_target = SVGAnimatedString::create(realm(), *this, DOM::QualifiedName { HTML::AttributeNames::target, OptionalNone {}, OptionalNone {} });
    return *m_target;
}

// https://svgwg.org/svg2-draft/linking.html#__svg__SVGAElement__relList
GC::Ref<DOM::DOMTokenList> SVGAElement::rel_list()
{
    // The relList IDL attribute reflects the ‘rel’ content attribute.
    if (!m_rel_list)
        m_rel_list = DOM::DOMTokenList::create(*this, HTML::AttributeNames::rel);
    return *m_rel_list;
}

GC::Ptr<Layout::Node> SVGAElement::create_layout_node(GC::Ref<CSS::ComputedProperties> style)
{
    return heap().allocate<Layout::SVGGraphicsBox>(document(), *this, move(style));
}

// TODO: move to SVGElement
// https://html.spec.whatwg.org/multipage/semantics.html#get-an-element's-target
String SVGAElement::get_an_elements_target(Optional<String> target) const
{
    // To get an element's target, given an a, area, or form element element, and an optional string-or-null target (default null), run these steps:

    // 1. If target is null, then:
    if (!target.has_value()) {
        // 1. If element has a target attribute, then set target to that attribute's value.
        if (auto maybe_target = attribute(AttributeNames::target); maybe_target.has_value()) {
            target = maybe_target.release_value();
        }
        // 2. Otherwise, if element's node document contains a base element with a target attribute,
        //    set target to the value of the target attribute of the first such base element.
        if (auto base_element = document().first_base_element_with_target_in_tree_order())
            target = base_element->attribute(AttributeNames::target);
    }

    // 2. If target is not null, and contains an ASCII tab or newline and a U+003C (<), then set target to "_blank".
    if (target.has_value() && target->bytes_as_string_view().contains("\t\n\r"sv) && target->contains('<'))
        target = "_blank"_string;

    // 3. Return target.
    return target.value_or({});
}

// https://html.spec.whatwg.org/multipage/links.html#get-an-element's-noopener
Web::HTML::TokenizedFeature::NoOpener SVGAElement::get_an_elements_noopener(URL::URL const& url, StringView target) const
{
    // To get an element's noopener, given an a, area, or form element element, a URL record url, and a string target,
    // perform the following steps. They return a boolean.
    auto rel = MUST(get_attribute_value(HTML::AttributeNames::rel).to_lowercase());
    auto link_types = rel.bytes_as_string_view().split_view_if(Web::Infra::is_ascii_whitespace);

    // 1. If element's link types include the noopener or noreferrer keyword, then return true.
    if (link_types.contains_slow("noopener"sv) || link_types.contains_slow("noreferrer"sv))
        return Web::HTML::TokenizedFeature::NoOpener::Yes;

    // 2. If element's link types do not include the opener keyword and
    //    target is an ASCII case-insensitive match for "_blank", then return true.
    if (!link_types.contains_slow("opener"sv) && target.equals_ignoring_ascii_case("_blank"sv))
        return Web::HTML::TokenizedFeature::NoOpener::Yes;

    // 3. If url's blob URL entry is not null:
    if (url.blob_url_entry().has_value()) {
        // 1. Let blobOrigin be url's blob URL entry's environment's origin.
        auto const& blob_origin = url.blob_url_entry()->environment.origin;

        // 2. Let topLevelOrigin be element's relevant settings object's top-level origin.
        auto const& top_level_origin = relevant_settings_object(*this).top_level_origin;

        // 3. If blobOrigin is not same site with topLevelOrigin, then return true.
        if (!blob_origin.is_same_site(top_level_origin.value()))
            return Web::HTML::TokenizedFeature::NoOpener::Yes;
    }

    // 4. Return false.
    return Web::HTML::TokenizedFeature::NoOpener::No;
}

}
