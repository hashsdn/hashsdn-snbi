package org.opendaylight.snbi.southplugin;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.sal.binding.api.AbstractBindingAwareProvider;
import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.ProviderContext;
import org.osgi.framework.BundleContext;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

// Activator class following the new MD-SAL path
public class Activator extends AbstractBindingAwareProvider {
	
	private DataBroker dataBroker;
	private static Activator INSTANCE;
	
	public static Activator getInstance() {
        return INSTANCE;
    }
	public DataBroker getDataBroker() {
        return dataBroker;
    }
	
    protected static final Logger logger = LoggerFactory
            .getLogger(Activator.class);

    // called during osgi start
    @Override
    public void onSessionInitiated(final ProviderContext session) {
        logger.debug("In onSessionInitiated");
        logger.debug("Initializing SNBI South Plugin Activator");
        try {
        	INSTANCE = this;
        	this.dataBroker = session.getSALService(DataBroker.class);
            SnbiInternal snbi = new SnbiInternal();
            snbi.start();
            CertManager.INSTANCE.start();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // called durng osgi stop
    @Override
    protected void stopImpl(final BundleContext context) {
        logger.info("SNBI South Plugin Activator clean up completed ");
    }

}
