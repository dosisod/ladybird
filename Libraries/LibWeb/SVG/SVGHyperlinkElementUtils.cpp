/*
 * Copyright (c) 2024, Andreas Kling <andreas@ladybird.org>
 * Copyright (c) 2024, Jamie Mansfield <jmansfield@cadixdev.org>
 * Copyright (c) 2025, Logan Hunt
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Forward.h>
#include <LibWeb/SVG/AttributeNames.h>
#include <LibWeb/SVG/SVGHyperlinkElementUtils.h>

namespace Web::SVG {

SVGHyperlinkElementUtils::SVGHyperlinkElementUtils(SVGElement& element, DOM::Document& document)
    : m_element(element)
    , m_document(document)
{
}

SVGHyperlinkElementUtils::~SVGHyperlinkElementUtils() = default;

Optional<String> SVGHyperlinkElementUtils::hyperlink_element_utils_href() const
{
    return m_element.attribute(AttributeNames::href);
}

void SVGHyperlinkElementUtils::set_hyperlink_element_utils_href(String href)
{
    m_element.set_attribute_value(AttributeNames::href, move(href));
}

Optional<String> SVGHyperlinkElementUtils::hyperlink_element_utils_referrerpolicy() const
{
    return m_element.attribute(AttributeNames::referrerpolicy);
}

}
