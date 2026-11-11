// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#include "qVersionResolve.h"

namespace sls {

int qResolve_GetQFontWidth(const QFontMetrics fm, const QString &text,
                           int len) {
    return fm.horizontalAdvance(text, len);
}

} // namespace sls
