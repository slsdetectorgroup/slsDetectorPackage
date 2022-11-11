// SPDX-License-Identifier: LGPL-3.0-or-other
// Copyright (C) 2021 Contributors to the SLS Detector Package
#pragma once

#include <QFontMetrics>
#include <QString>

namespace sls {

int qResolve_GetQFontWidth(const QFontMetrics fm, const QString &text,
                           int len = -1);

} // namespace sls
