//
// Created by andy on 5/1/2025.
//

#pragma once

#define ENTT_SINK_FOR(sigh, name) decltype(sigh)::sink_type name{sigh};
