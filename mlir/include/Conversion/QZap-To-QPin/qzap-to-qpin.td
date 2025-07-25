#ifndef QZAP_CONVERSION_TO_QPIN
#define QZAP_CONVERSION_TO_QPIN

include "mlir/Pass/PassBase.td"

def QZapToQPin : Pass<"qzap-to-qpin"> {
  let summary = "Lower `qzap` to `qpin`";
  let description = [{}];
  let dependentDialects = [
    "aqomplice::qpin::QPinDialect"
  ];
}

#endif  // QZAP_CONVERSION_TO_QPIN