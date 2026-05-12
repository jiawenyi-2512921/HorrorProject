# Bugfix Workflow

## Rule

No fixes without root cause. A patch that hides symptoms is not accepted.

## Steps

1. Reproduce or document why reproduction is blocked.
2. Read exact error/log/output.
3. Trace data flow to source.
4. Compare with working examples.
5. State one hypothesis.
6. Create or identify regression coverage.
7. Fix root cause.
8. Verify with the smallest relevant command.
9. Run broader verification if risk warrants it.
10. Record residual risk.

## Stop Conditions

- Three failed fix attempts.
- Architecture appears wrong.
- Verification cannot be run.
- Another agent owns the affected file.

