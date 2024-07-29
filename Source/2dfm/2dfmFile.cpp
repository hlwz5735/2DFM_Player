#include "2dfmFile.hpp"

void _2dfm::freeCommonResourcePart(_2dfm::CommonResourcePart *crp) {
    if (crp->rawScriptsData) {
        free(crp->rawScriptsData);
    }
    if (crp->scriptItems) {
        free(crp->scriptItems);
    }
    for (auto p : crp->pictures) {
        free(p);
    }
    crp->pictures.clear();
    for (auto p : crp->sharedPalettes) {
        free(p);
    }
    for (auto s : crp->sounds) {
        free(s);
    }
    crp->sounds.clear();
}
