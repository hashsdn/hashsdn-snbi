/*
 * Copyright (c) 2014 Ericsson Systems, Inc. and others.  All rights reserved.
 * Anu Nair
 * anu.nair@ericsson.com
 */

package org.opendaylight.snbi.certmgmt;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;

import org.bouncycastle.asn1.x500.AttributeTypeAndValue;
import org.bouncycastle.asn1.x500.RDN;
import org.bouncycastle.asn1.x500.X500Name;
import org.bouncycastle.asn1.x500.X500NameBuilder;
import org.bouncycastle.asn1.x500.style.BCStyle;
import org.bouncycastle.asn1.x509.Certificate;
import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cert.X509v3CertificateBuilder;
import org.bouncycastle.cert.jcajce.JcaX509CertificateConverter;
import org.bouncycastle.cert.jcajce.JcaX509CertificateHolder;
import org.bouncycastle.cert.jcajce.JcaX509v3CertificateBuilder;
import org.bouncycastle.operator.ContentSigner;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.operator.jcajce.JcaContentSignerBuilder;
import org.bouncycastle.pkcs.PKCS10CertificationRequest;
import org.bouncycastle.pkcs.PKCS10CertificationRequestBuilder;
import org.bouncycastle.pkcs.jcajce.JcaPKCS10CertificationRequestBuilder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public enum SNBICAInterfaces {
    INSTANCE;
    protected static final Logger logger = LoggerFactory
            .getLogger(SNBICAInterfaces.class);

    /*   API's called / requested by other modules */

    // generate a signed certificate signing request
    // order of arguments, common name, domain name, UDI (Serial Number)
    public PKCS10CertificationRequest generateCSRRequest(String... arguments) {
        X500NameBuilder builder = new X500NameBuilder(BCStyle.INSTANCE);
        builder.addRDN(BCStyle.CN, arguments[0]); // common name
        builder.addRDN(BCStyle.OU, arguments[1]); //  organisational unit
        builder.addRDN(BCStyle.SN, arguments[2]); // serial number
        // other defaults
        builder.addRDN(BCStyle.C, CertificateMgmt.defaults.get("COUNTRY"));
        builder.addRDN(BCStyle.ST, CertificateMgmt.defaults.get("STATE"));
        builder.addRDN(BCStyle.T, CertificateMgmt.defaults.get("TITLE"));

        //generate key pair
        KeyPair keyPair = KeyPairMgmt
                .generateKeyPair(CertManagerConstants.ALGORITHM.RSA);

        JcaContentSignerBuilder csBuilder = new JcaContentSignerBuilder(CertManagerConstants.CERT_ALGORITHM.SHA256WithRSAEncryption.toString());
        ContentSigner signer = null;
        try {
            signer = csBuilder.build(keyPair.getPrivate());
        } catch (OperatorCreationException e) {
            e.printStackTrace();
            return null;
        }
        PKCS10CertificationRequestBuilder p10Builder =  new JcaPKCS10CertificationRequestBuilder(
                builder.build(), keyPair.getPublic());

        PKCS10CertificationRequest csr = p10Builder.build(signer);
        return csr;
    }

    // generate X509 certificate from the CSR and signer which contains the signature.
    // certificate validity is for 3 years from now
    // extract the serial number from csr and use signature from csr or create one
    public X509Certificate generateX509Certificate(PKCS10CertificationRequest request, ContentSigner signer) {
        X509Certificate rootCert = CertificateMgmt
                .getSavedCertificate(CertManagerConstants.BC,CertManagerConstants.SELF_SIGNED_CERT_FILE);
        KeyPair rootPair = KeyPairMgmt.getKeyPairFromStore(CertManagerConstants.KEY_STORE_ALIAS,CertManagerConstants.KEY_STORE_CERT_ALIAS,  CertManagerConstants.STORE_TYPE.JKS);

        X500Name x500Name = request.getSubject();
        RDN cn = x500Name.getRDNs(BCStyle.SN)[0];
        AttributeTypeAndValue[] values = cn.getTypesAndValues();
        BigInteger serial = BigInteger.valueOf(new Long(values[0].getValue().toString()).longValue());
        Calendar now = Calendar.getInstance();
        Date notBefore = now.getTime();
        now.add(Calendar.YEAR, 3);
        Date notAfter = now.getTime();

        X509v3CertificateBuilder certGen = new JcaX509v3CertificateBuilder(
                request.getSubject(), serial, notBefore, notAfter,
                request.getSubject(), rootCert.getPublicKey());
        if (signer == null) {
            try {
                signer = new JcaContentSignerBuilder(
                        CertManagerConstants.CERT_ALGORITHM.SHA256WithRSAEncryption.toString()).setProvider(CertManagerConstants.BC
                                ).build(rootPair.getPrivate());
            } catch (OperatorCreationException e) {
                e.printStackTrace();
                return null;
            }
        }
        try {
            X509Certificate issuedCert = new JcaX509CertificateConverter().setProvider(
                    CertManagerConstants.BC).getCertificate(
                            certGen.build(signer));
            return issuedCert;
        } catch (CertificateException e) {
            e.printStackTrace();
            return null;
        }
    }

    // compare two X509 certificates. use the equals method defined in X509CertificateHolder
    public boolean compareCertificates(Certificate cert1, Certificate cert2) {
        return new X509CertificateHolder (cert1).equals(new X509CertificateHolder (cert2));
    }

    // returns the basic information about the certificate in a hashmap
    public HashMap<String,String> getCertificateInfo(X509Certificate cert){
        HashMap<String,String> certInfo = new HashMap<String,String>();
        X500Name x500name;
        try {
            x500name = new JcaX509CertificateHolder(cert).getSubject();
        } catch (CertificateEncodingException e) {
            e.printStackTrace();
            return certInfo;
        }
        certInfo.put(CertManagerConstants.SUBJECT_NAME,cert.getSubjectDN().getName());
        certInfo.put(CertManagerConstants.ISSUER_NAME,cert.getIssuerDN().getName());
        certInfo.put(CertManagerConstants.SN, cert.getSerialNumber().toString());
        RDN cn = x500name.getRDNs(BCStyle.CN)[0];
        certInfo.put(CertManagerConstants.CN,cn.toString());
        RDN ou = x500name.getRDNs(BCStyle.OU)[0];
        certInfo.put(CertManagerConstants.OU,ou.toString());
        certInfo.put(CertManagerConstants.START_DATE,cert.getNotBefore().toString());
        certInfo.put(CertManagerConstants.EXPIRY_DATE,cert.getNotAfter().toString());
        return certInfo;
    }

    // save the certificate in keystore file and returns the alias for reference
    public String saveCertificate(X509Certificate cert ) {
        KeyStore keystore = null;
        String certAlias = CertManagerConstants.KEY_STORE_CERT_ALIAS+SNBIRegistrar.ID++;
        try {
            keystore = KeyStore.getInstance(CertManagerConstants.STORE_TYPE.JCEKS.toString());
        } catch (KeyStoreException e) {
            e.printStackTrace();
            return null;
        }
        try (FileInputStream is = new FileInputStream(
                CertManagerConstants.KEY_STORE_FILE)) {
            keystore.load(is,
                    CertManagerConstants.KEY_STORE_PASSWORD.toCharArray());
        } catch (NoSuchAlgorithmException | CertificateException | IOException e) {
            e.printStackTrace();
            return null;
        }
        try {
            keystore.setCertificateEntry(
                    certAlias, cert);
        } catch (KeyStoreException e) {
            e.printStackTrace();
            return null;
        }
        try {
            keystore.store(new FileOutputStream(
                    CertManagerConstants.KEY_STORE_FILE),
                    CertManagerConstants.KEY_STORE_PASSWORD.toCharArray());
        } catch (KeyStoreException | NoSuchAlgorithmException
                | CertificateException | IOException e) {
            e.printStackTrace();
            return null;
        }
        return certAlias;
    }

    // retreive the saved certificate
    public X509Certificate getSavedCertificate(String alias) {
        KeyStore keystore = null;
        try {
            keystore = KeyStore.getInstance(CertManagerConstants.STORE_TYPE.JCEKS.toString());
        } catch (KeyStoreException e) {
            e.printStackTrace();
            return null;
        }
        try (FileInputStream is = new FileInputStream(
                CertManagerConstants.KEY_STORE_FILE)) {
            keystore.load(is,
                    CertManagerConstants.KEY_STORE_PASSWORD.toCharArray());
        } catch (NoSuchAlgorithmException | CertificateException | IOException e) {
            e.printStackTrace();
            return null;
        }
        try {
            return (X509Certificate) keystore.getCertificate(alias);
        } catch (KeyStoreException e) {
            e.printStackTrace();
            return null;
        }
    }

    // get the root certificate
    public X509Certificate getRootCACertificate() {
        return CertificateMgmt.getSavedCertificate(
                CertManagerConstants.BC,
                CertManagerConstants.SELF_SIGNED_CERT_FILE);
    }

    // verify the certificate for date and public key
    public String verifyCertificate(X509Certificate cert, Date date, PublicKey pub_key) {
        return CertificateMgmt.verifyCertificate(cert, date, pub_key);
    }

    // generate the signature
    public byte[] generateSignature(byte[] data, Certificate cert,String algorithm) {
        return CertificateMgmt.generateSignature(data, cert, algorithm);
    }

    // verify the signature on certificate
    public  boolean verifySignature(byte[] data,byte[] hash,Certificate cert,String algorithm) {
        return CertificateMgmt.verifySignature(data, hash, cert, algorithm);
    }

}