#include "skrillex/dbo.hpp"

namespace skrillex {
    bool operator==(const Artist& a, const Artist& b) {
        return a.id == b.id && a.session_id == b.session_id;
    }

    bool operator==(const Genre& a, const Genre& b) {
        return a.id == b.id && a.session_id == b.session_id;
    }

    bool operator==(const Song& a, const Song& b) {
        return
            a.artist == b.artist &&
            a.genre == b.genre &&
            a.id == b.id && a.session_id == b.session_id;
    }
}

