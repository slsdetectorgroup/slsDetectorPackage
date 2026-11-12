// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qVersionResolve.h"

namespace sls {

int qResolve_GetQFontWidth(const QFontMetrics fm, const QString &text,
                           int len) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(text, len);
#else
    // RHEL 7 still ships Qt 5.9, where QFontMetrics::width() is the only
    // available overload for this.
    return fm.width(text, len);
#endif
}

} // namespace sls
