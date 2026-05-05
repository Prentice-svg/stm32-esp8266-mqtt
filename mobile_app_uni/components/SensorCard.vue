<template>
	<view class="sensor-card">
		<StatusBadge class="status-tag" :text="status" type="success" />
		<view class="icon-wrap">
			<image v-if="iconPath" class="icon-image" :src="iconPath" mode="aspectFit" />
			<text v-else class="icon-text">{{ icon }}</text>
		</view>
		<text class="sensor-name">{{ name }}</text>
		<view class="divider"></view>
		<view class="sensor-bottom" :class="{ control: switchable }">
			<view class="value-wrap">
				<text class="sensor-value" :class="tone">{{ displayValue }}</text>
				<text v-if="unit" class="sensor-unit">{{ unit }}</text>
			</view>
			<switch
				v-if="switchable"
				class="control-switch"
				:checked="checked"
				color="#22C55E"
				@change="$emit('toggle', $event.detail.value)"
			/>
		</view>
	</view>
</template>

<script>
import StatusBadge from './StatusBadge.vue'

export default {
	name: 'SensorCard',
	components: {
		StatusBadge
	},
	props: {
		name: {
			type: String,
			required: true
		},
		value: {
			type: [String, Number, Boolean],
			default: '--'
		},
		unit: {
			type: String,
			default: ''
		},
		status: {
			type: String,
			default: '正常'
		},
		icon: {
			type: String,
			default: '●'
		},
		iconPath: {
			type: String,
			default: ''
		},
		tone: {
			type: String,
			default: 'blue'
		},
		switchable: {
			type: Boolean,
			default: false
		},
		checked: {
			type: Boolean,
			default: false
		}
	},
	emits: ['toggle'],
	computed: {
		displayValue() {
			if (this.switchable) {
				return this.checked ? '开启' : '关闭'
			}
			return this.value
		}
	}
}
</script>

<style>
.sensor-card {
	position: relative;
	min-height: 304rpx;
	padding: 22rpx 22rpx 24rpx;
	border-radius: 28rpx;
	background: #ffffff;
	box-shadow: 0 14rpx 32rpx rgba(15, 23, 42, 0.09);
	display: flex;
	flex-direction: column;
	align-items: center;
	overflow: hidden;
	border: 1rpx solid rgba(226, 232, 240, 0.72);
	box-sizing: border-box;
}

.status-tag {
	position: absolute;
	right: 16rpx;
	top: 16rpx;
	transform: scale(0.82);
	transform-origin: right top;
}

.icon-wrap {
	width: 132rpx;
	height: 114rpx;
	margin-top: 22rpx;
	display: flex;
	align-items: center;
	justify-content: center;
}

.icon-image {
	width: 132rpx;
	height: 114rpx;
}

.icon-text {
	font-size: 58rpx;
	line-height: 1;
}

.divider {
	width: 100%;
	height: 2rpx;
	margin: 16rpx 0 12rpx;
	background: #E2E8F0;
}

.sensor-name {
	display: block;
	margin-top: 10rpx;
	min-height: 34rpx;
	font-size: 28rpx;
	line-height: 1.25;
	font-weight: 900;
	color: #0F172A;
	text-align: center;
}

.sensor-bottom {
	width: 100%;
	display: flex;
	align-items: center;
	justify-content: center;
	gap: 12rpx;
}

.sensor-bottom.control {
	flex-direction: column;
	gap: 8rpx;
}

.value-wrap {
	display: flex;
	align-items: baseline;
	gap: 8rpx;
	min-width: 0;
}

.sensor-value {
	font-size: 48rpx;
	line-height: 1;
	font-weight: 900;
	color: #0F172A;
}

.sensor-value.blue {
	color: #2563EB;
}

.sensor-value.teal,
.sensor-value.green {
	color: #14B8A6;
}

.sensor-value.warning {
	color: #F59E0B;
}

.sensor-unit {
	font-size: 22rpx;
	font-weight: 650;
	color: #334155;
}

.control-switch {
	transform: scale(0.82);
	transform-origin: center;
}
</style>
