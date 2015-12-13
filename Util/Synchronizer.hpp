#pragma once
/**
 * @date   Jul 17, 2013
 * @author Sam Roth <>
 */

namespace dbuilder
{

/**
 * Solves the cyclic-signal-slot madness.
 */
class Synchronizer: public QObject
{
	Q_OBJECT
	QObject *_syncSender;
public:
	Synchronizer(QObject *parent=nullptr);
	virtual ~Synchronizer();

	/**
	 * @return The invoker of the viewUpdate() or modelUpdate()
	 * slot if it was invoked by a signal. Otherwise, returns this.
	 */
	QObject *syncSender();

	/**
	 * Convenience method: connects receiver's
	 * viewUpdatedSlot to viewUpdated() and modelUpdatedSlot
	 * to modelUpdated().
	 */
	Synchronizer *whenUpdated(QObject *receiver,
	                          const char *viewUpdatedSlot,
	                          const char *modelUpdatedSlot);


public slots:
	/**
	 * Call when the view is updated.
	 */
	void viewUpdate();
	/**
	 * Call when the model is updated.
	 */
	void modelUpdate();
signals:
	/**
	 * Emitted after view is updated, but not due to a model update.
	 */
	void viewUpdated();
	/**
	 * Emitted after model is updated, but not due to a view update.
	 */
	void modelUpdated();
};

} /* namespace dbuilder */
