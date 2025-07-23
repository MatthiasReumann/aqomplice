#ifndef QZAP_CONVERSION_TO_QPIN
#define QZAP_CONVERSION_TO_QPIN

include "mlir/Pass/PassBase.td"

def QZapToQPin : Pass<"qzap-to-qpin"> {
  let summary = "Lower `qzap` to `qpin`";

  let description = [{
    This pass lowers the `qzap` dialect to `qpin`.
  }];

  let dependentDialects = [
    "aqomplice::qpin::QPinDialect"
  ];
}

#endif  // QZAP_CONVERSION_TO_QPIN