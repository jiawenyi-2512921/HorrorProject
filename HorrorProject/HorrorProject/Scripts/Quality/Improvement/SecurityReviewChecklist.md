# Security Review Checklist

## Overview
Security-focused checklist for code reviews to identify and prevent security vulnerabilities.

**Version**: 1.0
**Last Updated**: 2026-04-26
**Severity Scale**: Critical > High > Medium > Low

---

## Input Validation

### User Input
- [ ] All user input is validated before use
- [ ] Input length limits enforced
- [ ] Input type checking performed
- [ ] Special characters handled safely
- [ ] Whitelist validation used where possible
- [ ] Input sanitization applied

### Network Input
- [ ] Network data validated before processing
- [ ] Packet size limits enforced
- [ ] Malformed data handled gracefully
- [ ] Protocol validation implemented
- [ ] Rate limiting in place

### File Input
- [ ] File paths validated (no directory traversal)
- [ ] File size limits enforced
- [ ] File type validation performed
- [ ] Malicious file content checked
- [ ] Safe file parsing used

---

## Authentication & Authorization

### Authentication
- [ ] Strong authentication mechanisms used
- [ ] No hardcoded credentials
- [ ] Credentials stored securely
- [ ] Session management secure
- [ ] Multi-factor authentication supported (if applicable)

### Authorization
- [ ] Proper access control checks
- [ ] Principle of least privilege followed
- [ ] Authorization checked on server side
- [ ] No client-side only authorization
- [ ] Role-based access control implemented

### Session Management
- [ ] Session tokens are random and unpredictable
- [ ] Sessions expire appropriately
- [ ] Session invalidation on logout
- [ ] Concurrent session handling
- [ ] Session fixation prevented

---

## Data Protection

### Sensitive Data
- [ ] Sensitive data identified and classified
- [ ] Encryption used for sensitive data at rest
- [ ] Encryption used for sensitive data in transit
- [ ] No sensitive data in logs
- [ ] No sensitive data in error messages
- [ ] Secure deletion of sensitive data

### Passwords & Secrets
- [ ] No hardcoded passwords or API keys
- [ ] Passwords hashed with strong algorithm
- [ ] Salts used for password hashing
- [ ] Secrets stored in secure configuration
- [ ] No secrets in version control
- [ ] Environment variables used for secrets

### Personal Information
- [ ] PII handling complies with regulations
- [ ] Data minimization practiced
- [ ] User consent obtained where required
- [ ] Data retention policies followed
- [ ] Secure data disposal

---

## Memory Safety

### Buffer Management
- [ ] No buffer overflows
- [ ] Array bounds checked
- [ ] String operations safe (no strcpy, sprintf)
- [ ] Dynamic memory properly managed
- [ ] No use-after-free vulnerabilities

### Pointer Safety
- [ ] Null pointer checks before dereferencing
- [ ] No dangling pointers
- [ ] Smart pointers used where appropriate
- [ ] No double-free vulnerabilities
- [ ] Proper ownership semantics

### Resource Management
- [ ] Resources properly released
- [ ] RAII pattern used
- [ ] No resource leaks
- [ ] Exception-safe resource handling
- [ ] Proper cleanup in error paths

---

## Injection Attacks

### SQL Injection (if applicable)
- [ ] Parameterized queries used
- [ ] No string concatenation for SQL
- [ ] Input validation for database operations
- [ ] Least privilege database accounts
- [ ] Stored procedures used where appropriate

### Command Injection
- [ ] No system command execution with user input
- [ ] Command parameters validated
- [ ] Whitelist approach for allowed commands
- [ ] Safe APIs used instead of shell commands
- [ ] Input sanitization for commands

### Code Injection
- [ ] No dynamic code execution with user input
- [ ] Eval-like functions avoided
- [ ] Script injection prevented
- [ ] Safe deserialization practices
- [ ] Content Security Policy implemented (if web)

---

## Cryptography

### Encryption
- [ ] Strong encryption algorithms used (AES-256, etc.)
- [ ] No deprecated algorithms (DES, MD5, SHA1)
- [ ] Proper key management
- [ ] Secure random number generation
- [ ] Initialization vectors properly used

### Hashing
- [ ] Cryptographic hash functions used correctly
- [ ] No weak hash algorithms
- [ ] Salts used for password hashing
- [ ] Proper hash comparison (timing-safe)
- [ ] Hash algorithms appropriate for use case

### Key Management
- [ ] Keys stored securely
- [ ] Key rotation implemented
- [ ] No hardcoded keys
- [ ] Proper key generation
- [ ] Key access controlled

---

## Network Security

### Communication
- [ ] TLS/SSL used for sensitive data
- [ ] Certificate validation performed
- [ ] No insecure protocols (HTTP, FTP, Telnet)
- [ ] Secure protocol versions used
- [ ] Man-in-the-middle attacks prevented

### API Security
- [ ] API authentication required
- [ ] API rate limiting implemented
- [ ] API input validation
- [ ] API versioning handled securely
- [ ] CORS configured properly (if applicable)

### Replication (Unreal Specific)
- [ ] Server authority enforced
- [ ] Client input validated on server
- [ ] No trust in client data
- [ ] Cheat prevention measures
- [ ] Secure RPC implementation

---

## Error Handling & Logging

### Error Messages
- [ ] No sensitive information in error messages
- [ ] Generic error messages to users
- [ ] Detailed errors logged securely
- [ ] Stack traces not exposed to users
- [ ] Error codes used appropriately

### Logging
- [ ] Security events logged
- [ ] No sensitive data in logs
- [ ] Logs protected from tampering
- [ ] Log injection prevented
- [ ] Appropriate log retention

### Exception Handling
- [ ] All exceptions caught and handled
- [ ] No information leakage through exceptions
- [ ] Resources cleaned up on exception
- [ ] Security context maintained
- [ ] Fail securely

---

## Unreal Engine Specific

### Replication Security
- [ ] Server validates all client RPCs
- [ ] No client authority on critical data
- [ ] Replicated properties validated
- [ ] Anti-cheat measures implemented
- [ ] Movement validation on server

### Blueprint Security
- [ ] Blueprint-exposed functions validated
- [ ] No security-critical logic in Blueprints only
- [ ] Blueprint input validation
- [ ] Proper access control in Blueprints
- [ ] Console commands secured

### Asset Security
- [ ] Asset loading validated
- [ ] No arbitrary asset loading from user input
- [ ] Asset paths sanitized
- [ ] Pak file integrity checked
- [ ] DLC security considered

### Console Commands
- [ ] Console commands require authorization
- [ ] Cheat commands disabled in shipping
- [ ] Command input validated
- [ ] No sensitive operations via console
- [ ] Command logging implemented

---

## Common Vulnerabilities

### OWASP Top 10 (Adapted for Games)

#### 1. Injection
- [ ] No SQL, command, or code injection vulnerabilities
- [ ] Input validation and sanitization
- [ ] Parameterized queries used

#### 2. Broken Authentication
- [ ] Strong authentication mechanisms
- [ ] Session management secure
- [ ] Credential storage secure

#### 3. Sensitive Data Exposure
- [ ] Encryption for sensitive data
- [ ] No sensitive data in logs or errors
- [ ] Secure data transmission

#### 4. XML External Entities (XXE)
- [ ] XML parsing configured securely
- [ ] External entity processing disabled
- [ ] Input validation for XML

#### 5. Broken Access Control
- [ ] Authorization checks enforced
- [ ] Server-side validation
- [ ] Principle of least privilege

#### 6. Security Misconfiguration
- [ ] Secure default configurations
- [ ] Unnecessary features disabled
- [ ] Security headers configured
- [ ] Error handling configured securely

#### 7. Cross-Site Scripting (XSS)
- [ ] Output encoding implemented
- [ ] Input validation
- [ ] Content Security Policy (if web UI)

#### 8. Insecure Deserialization
- [ ] Deserialization input validated
- [ ] Type checking on deserialized objects
- [ ] No arbitrary code execution

#### 9. Using Components with Known Vulnerabilities
- [ ] Dependencies up to date
- [ ] Vulnerability scanning performed
- [ ] Third-party libraries vetted

#### 10. Insufficient Logging & Monitoring
- [ ] Security events logged
- [ ] Monitoring in place
- [ ] Alerting configured
- [ ] Audit trail maintained

---

## Game-Specific Security

### Cheating Prevention
- [ ] Server authority for game state
- [ ] Client input validation
- [ ] Anti-cheat measures implemented
- [ ] Memory protection considered
- [ ] Packet encryption used

### Economy & Transactions
- [ ] Transaction validation on server
- [ ] No client-side currency manipulation
- [ ] Audit logging for transactions
- [ ] Rollback mechanisms in place
- [ ] Fraud detection implemented

### Player Data
- [ ] Player data encrypted
- [ ] Save game integrity checked
- [ ] No save game manipulation
- [ ] Cloud save security
- [ ] Data backup and recovery

### Multiplayer Security
- [ ] Server validates all actions
- [ ] No trust in client timing
- [ ] Lag compensation secure
- [ ] DDoS protection considered
- [ ] Player reporting system

---

## Code Analysis

### Static Analysis
- [ ] Static analysis tools run
- [ ] Security warnings addressed
- [ ] Code complexity analyzed
- [ ] Dependency vulnerabilities checked
- [ ] Code coverage adequate

### Dynamic Analysis
- [ ] Fuzzing performed on inputs
- [ ] Penetration testing conducted
- [ ] Runtime security checks
- [ ] Memory sanitizers used
- [ ] Performance under attack tested

---

## Threat Modeling

### Assets
- [ ] Critical assets identified
- [ ] Asset value assessed
- [ ] Asset protection appropriate

### Threats
- [ ] Threat actors identified
- [ ] Attack vectors analyzed
- [ ] Threat likelihood assessed
- [ ] Impact evaluated

### Mitigations
- [ ] Mitigations implemented
- [ ] Defense in depth applied
- [ ] Fail-safe defaults used
- [ ] Security by design

---

## Security Testing

### Unit Tests
- [ ] Security test cases included
- [ ] Boundary conditions tested
- [ ] Negative test cases included
- [ ] Error conditions tested

### Integration Tests
- [ ] End-to-end security testing
- [ ] Authentication flow tested
- [ ] Authorization tested
- [ ] Data flow security verified

### Penetration Testing
- [ ] Manual security testing performed
- [ ] Automated security scans run
- [ ] Vulnerability assessment done
- [ ] Findings addressed

---

## Compliance & Standards

### Regulatory Compliance
- [ ] GDPR compliance (if applicable)
- [ ] COPPA compliance (if applicable)
- [ ] Regional regulations followed
- [ ] Privacy policy implemented
- [ ] Terms of service clear

### Industry Standards
- [ ] OWASP guidelines followed
- [ ] CWE/SANS Top 25 addressed
- [ ] Platform security requirements met
- [ ] Best practices implemented

---

## Security Review Process

### Pre-Review
- [ ] Threat model reviewed
- [ ] Security requirements understood
- [ ] Previous vulnerabilities checked
- [ ] Security tools run

### During Review
- [ ] Focus on security-critical code
- [ ] Check for common vulnerabilities
- [ ] Verify input validation
- [ ] Review authentication/authorization
- [ ] Check cryptography usage

### Post-Review
- [ ] Security findings documented
- [ ] Severity assigned to issues
- [ ] Remediation plan created
- [ ] Follow-up scheduled
- [ ] Knowledge shared

---

## Severity Classification

### Critical
- Remote code execution
- Authentication bypass
- Privilege escalation
- Data breach potential
- System compromise

### High
- Sensitive data exposure
- Authorization bypass
- Injection vulnerabilities
- Cryptographic failures
- Session hijacking

### Medium
- Information disclosure
- Denial of service
- Weak authentication
- Insecure configuration
- Missing security headers

### Low
- Security through obscurity
- Verbose error messages
- Missing security best practices
- Weak password policy
- Insufficient logging

---

## Security Checklist by Component

### Network Code
- [ ] Server authority enforced
- [ ] Input validation on server
- [ ] Rate limiting implemented
- [ ] Encryption used
- [ ] DDoS protection

### Save System
- [ ] Save data integrity checked
- [ ] Encryption for sensitive saves
- [ ] No arbitrary code in saves
- [ ] Version checking
- [ ] Backup mechanism

### User Interface
- [ ] Input validation
- [ ] No script injection
- [ ] Secure data display
- [ ] CSRF protection (if web)
- [ ] Clickjacking prevention

### Asset Loading
- [ ] Path validation
- [ ] No arbitrary file access
- [ ] Asset integrity checked
- [ ] Secure deserialization
- [ ] Resource limits enforced

---

## Red Flags

### Code Patterns
- 🚨 `strcpy`, `sprintf`, `gets` (use safe alternatives)
- 🚨 `system()`, `exec()` with user input
- 🚨 Hardcoded credentials or keys
- 🚨 Disabled security checks
- 🚨 TODO: Add security check
- 🚨 Trust client data without validation
- 🚨 Weak random number generation
- 🚨 Deprecated cryptographic functions

### Comments
- 🚨 "Security check disabled for testing"
- 🚨 "TODO: Validate this input"
- 🚨 "Temporary workaround"
- 🚨 "This should be secure enough"
- 🚨 "Client-side validation only"

---

## Security Tools

### Static Analysis
- PVS-Studio
- Clang Static Analyzer
- Coverity
- SonarQube
- Visual Studio Code Analysis

### Dynamic Analysis
- Valgrind
- AddressSanitizer
- MemorySanitizer
- ThreadSanitizer
- UndefinedBehaviorSanitizer

### Dependency Scanning
- OWASP Dependency-Check
- Snyk
- WhiteSource
- Black Duck

---

## Incident Response

### Preparation
- [ ] Incident response plan exists
- [ ] Security contacts identified
- [ ] Escalation procedures defined
- [ ] Communication plan ready

### Detection
- [ ] Monitoring in place
- [ ] Alerting configured
- [ ] Log analysis automated
- [ ] Anomaly detection

### Response
- [ ] Incident classification process
- [ ] Containment procedures
- [ ] Evidence preservation
- [ ] Stakeholder communication

### Recovery
- [ ] Restoration procedures
- [ ] Verification process
- [ ] Post-incident review
- [ ] Lessons learned documented

---

## Resources

- [OWASP Top 10](https://owasp.org/www-project-top-ten/)
- [CWE Top 25](https://cwe.mitre.org/top25/)
- [SANS Security Guidelines](https://www.sans.org/security-resources/)
- [Unreal Engine Security Best Practices](https://docs.unrealengine.com/)
- [C++ Secure Coding](https://wiki.sei.cmu.edu/confluence/display/cplusplus)

---

## Security Review Sign-Off

**Reviewer**: _______________
**Date**: _______________
**Severity of Issues Found**: Critical: ___ High: ___ Medium: ___ Low: ___
**Recommendation**: ☐ Approve ☐ Approve with Conditions ☐ Reject

**Notes**:
_______________________________________________
_______________________________________________

---

**Remember**: Security is not a one-time check but an ongoing process. Stay informed about new vulnerabilities and continuously improve security practices.
