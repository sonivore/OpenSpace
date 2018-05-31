import React from 'react';
import PropTypes from 'prop-types';
import { connect } from 'react-redux';
import { setActivated } from '../../api/Actions/dataLoaderActions';

import styles from './ToggleDataLoader.scss';

let ToggleDataLoader = (props) => {
  const { activated } = props;

  return (
    <div className={`${styles.loader}`} onClick={() => props.setActivated(!activated)}>
      { activated ? 'Hide data loader' : 'Show data loader'}
    </div>
  );
};

ToggleDataLoader.propTypes = {
  activated: PropTypes.bool
};

ToggleDataLoader.defaultProps = {
  activated: false
};

const mapStateToProps = state => ({
  activated: state.dataLoader.activated
});

const mapDispatchToProps = dispatch => ({
  setActivated: (activated) => {
    dispatch(setActivated(activated));
  }
});

ToggleDataLoader = connect(
  mapStateToProps,
  mapDispatchToProps
)(ToggleDataLoader);

export default ToggleDataLoader;
