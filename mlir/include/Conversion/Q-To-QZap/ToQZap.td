#ifndef Q_CONVERSION_TO_QZAP
#define Q_CONVERSION_TO_QZAP

include "mlir/Pass/PassBase.td"

def QToQZap : Pass<"q-to-qzap"> {
  let summary = "Lower `q` to `qzap`";

  let description = [{
    This pass lowers the `q` dialect to `qzap`.
  }];

  let dependentDialects = [
    "aqomplice::qzap::QZapDialect"
  ];
}

#endif  // Q_CONVERSION_TO_QZAP