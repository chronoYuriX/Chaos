$cert = New-SelfSignedCertificate `
  -Type CodeSigningCert `
  -Subject "CN=YourName Dev Cert" `
  -KeyAlgorithm RSA -KeyLength 2048 `
  -HashAlgorithm SHA256 `
  -CertStoreLocation "Cert:\CurrentUser\My" `
  -NotAfter (Get-Date).AddYears(2)